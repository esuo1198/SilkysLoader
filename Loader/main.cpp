#define WIN32_LEAN_AND_MEAN
#include <filesystem>
#include <windows.h>
#include <detours.h>

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	if (__argc < 2) return 1;

	const auto pathArg = std::filesystem::path(__argv[1]);
	std::string strApp;
	std::string strAppDir;

	if (!pathArg.is_absolute()) {
		strApp = absolute(pathArg).string();
		strAppDir = pathArg.parent_path().string();
	}
	else {
		strApp = pathArg.string();
		strAppDir = pathArg.parent_path().string();
	}

	char szExe[MAX_PATH];
	GetModuleFileNameA(hInstance, szExe, sizeof(szExe));

	auto pathExe = std::filesystem::path(szExe);
	auto pathExeDir = pathExe.parent_path();

	auto strDll = (pathExeDir / "silkys.patch.dll").string();

	STARTUPINFOA startupInfo = { sizeof(startupInfo) };
	PROCESS_INFORMATION processInfo = {};

	if (DetourCreateProcessWithDllA(strApp.c_str(), NULL, NULL, NULL, FALSE, NULL, NULL, strAppDir.c_str(), &startupInfo, &processInfo, strDll.c_str(), NULL) != TRUE) {
		MessageBoxA(NULL, "DetourCreateProcessWithDllA failed", "SilkysLoader", MB_OK | MB_ICONERROR);
		return 1;
	}

	return 0;
}
