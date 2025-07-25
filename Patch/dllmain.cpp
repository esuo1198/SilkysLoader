#include "xml.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <shlobj.h>
#include <windows.h>
#include <detours.h>

#define DllExport __declspec(dllexport)

HMODULE hAppModule = NULL;
HMODULE hThisModule = NULL;

char szAppData[MAX_PATH];
char szCompanyName[256];
char szProductName[256];

void CreateConsole(void) {
	FILE* fp;
	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
}

std::string
RedirectHandler(const std::string& originalFileName, const std::string& currentFileName) {
	auto pathOriginal = std::filesystem::path(originalFileName.c_str());
	if (!pathOriginal.is_absolute()) pathOriginal = absolute(pathOriginal);

	const auto pathOriginalDir = std::filesystem::path(szAppData) / szCompanyName / szProductName;
	const auto pathRedirectDir = std::filesystem::current_path();

	if (pathOriginal.string().find(pathOriginalDir.string()) == 0) {
		auto strNewPath = pathOriginal.string();
		strNewPath.replace(0, pathOriginalDir.string().length(), pathRedirectDir.string());

#ifdef _DEBUG
		std::cout << "Redirected: " << strNewPath << std::endl;
#endif

		return strNewPath;
	}

	return "";
}

PVOID pvCreateFileOrigA = NULL;
typedef HANDLE(WINAPI* CreateFileTypeA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

HANDLE WINAPI CreateFileHookA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES
	lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile) {
	const auto strOriginalPath = std::string(lpFileName);
	auto strCurrentPath = std::string(strOriginalPath);

	const auto strResult = RedirectHandler(strOriginalPath, strCurrentPath);
	if (strResult != "") strCurrentPath = strResult;

	return ((CreateFileTypeA)pvCreateFileOrigA)(strCurrentPath.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile);
}

void SetVariables() {
	SHGetSpecialFolderPathA(NULL, szAppData, CSIDL_APPDATA, 0);
	if (!LoadStringA(hAppModule, 108, szCompanyName, sizeof(szCompanyName))) {
		MessageBoxA(NULL, "LoadStringA failed (uID: 108)", "SilkysLoader", MB_OK | MB_ICONERROR);
		exit(1);
	}
	if (!LoadStringA(hAppModule, 106, szProductName, sizeof(szProductName))) {
		MessageBoxA(NULL, "LoadStringA failed (uID: 106)", "SilkysLoader", MB_OK | MB_ICONERROR);
		exit(1);
	}
}

void WriteXml() {
	char szApp[MAX_PATH];
	GetModuleFileNameA(hAppModule, szApp, sizeof(szApp));

	const auto pathApp = std::filesystem::path(szApp);
	const auto pathAppDir = pathApp.parent_path();

	const auto strXmlName = std::string(szProductName) + "_inst.xml";
	const auto strXmlPath = (pathAppDir / strXmlName).string();

	std::ofstream ofs(strXmlPath, std::ios::out | std::ios::binary);
	if (!ofs) {
		MessageBoxA(NULL, "Failed to open inst xml file", "SilkysLoader", MB_OK | MB_ICONERROR);
		exit(1);
	}

	Writer xml(ofs);
	xml.openElt("InstallSetting")
		.openElt("InstallPath")
		.content(pathAppDir.string().c_str())
		.closeElt()
		.openElt("RunExe")
		.content(pathApp.string().c_str())
		.closeElt();
	xml.closeAll();

	ofs.close();
}

void InstallHooks() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	pvCreateFileOrigA = DetourFindFunction("kernel32.dll", "CreateFileA");
	DetourAttach(&pvCreateFileOrigA, CreateFileHookA);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		DetourRestoreAfterWith();

		hAppModule = GetModuleHandleA(NULL);
		hThisModule = hModule;

		SetVariables();
		WriteXml();
		InstallHooks();

#ifdef _DEBUG 
		CreateConsole();
#endif

		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

DllExport void WINAPI Dummy() {}
