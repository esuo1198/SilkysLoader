# SilkysLoader  
**Silky's Engine / GSIWin Game Loader**

This redirects file access (e.g., save data) from `%APPDATA%` to the game’s local directory.

## How to Use  
1. Build the project.  
2. Drag and drop the target game’s executable file onto `silkys.loader.exe` located in `Build/Artifacts`.

### Prerequisites  
Before launching the loader, move the save data folder:  
`%APPDATA%\SilkysPlus\{ProductName}\save` → *(move to the game directory)*

---

> Based on [crskycode/LoaderFramework](https://github.com/crskycode/LoaderFramework)