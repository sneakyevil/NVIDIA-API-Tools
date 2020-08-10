#include "Includes.hpp"

const char* cConsole = "[sneakyevil.eu] NVIDIA API Tools";
void ToggleConsoleVisibility()
{
    static bool bHidden;
    ShowWindow(nGlobal::hWindowConsole, bHidden);
    if (bHidden)
    {
        ShowWindow(nGlobal::hWindowConsole, SW_RESTORE);
        SetForegroundWindow(nGlobal::hWindowConsole);
        SetFocus(nGlobal::hWindowConsole);
        SetActiveWindow(nGlobal::hWindowConsole);
    }
    SetPriorityClass(GetCurrentProcess(), bHidden ? NORMAL_PRIORITY_CLASS : IDLE_PRIORITY_CLASS);
    bHidden = !bHidden;
}

std::string HWND2EXE(HWND hInput)
{
    static DWORD dPID;
    GetWindowThreadProcessId(hInput, &dPID);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32; pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe32))
        {
            while (Process32Next(hSnap, &pe32))
            {
                if (pe32.th32ProcessID == dPID)
                {
                    CloseHandle(hSnap);
                    return pe32.szExeFile;
                }
            }
        }
        CloseHandle(hSnap);
    }
    return "";
}

void SetDVCLevel(int iLevel)
{
    static int iOldLevel = -1;
    if (iLevel == iOldLevel) return; 
    iOldLevel = iLevel;
    (*nGlobal::nAPI::SetDVCLevel)(nGlobal::nAPI::iHandle, 0, static_cast<int>(iLevel * 1.26)); // Since 63 is 100% vibrance and we have settings between 0 - 50 this is "perfect" math for that.
}

#define GREEN   10
#define RED     12
#define YELLOW  14
#define WHITE   15
void PrintfColor(WORD wColor, const char* pFormat, ...)
{
    SetConsoleTextAttribute(nGlobal::hOutputConsole, wColor);
    char cTemp[1024];
    char *pArgs;
    va_start(pArgs, pFormat);
    _vsnprintf_s(cTemp, sizeof(cTemp) - 1, pFormat, pArgs);
    va_end(pArgs);
    cTemp[sizeof(cTemp) - 1] = 0;
    printf(cTemp);
}

/* Most ugly registry functions. */
const char* cRegistryRunPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
bool bStartupRegistered()
{
    bool bExist = false;
    HKEY hTemp = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, cRegistryRunPath, 0, KEY_READ, &hTemp) == ERROR_SUCCESS)
    {
        DWORD dType, dData;
        bExist = RegQueryValueExA(hTemp, cConsole, 0, &dType, 0, &dData) == ERROR_SUCCESS;
        RegCloseKey(hTemp);
    }
    return bExist;
}

void StartupRegisterToggle()
{
    HKEY hTemp = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, cRegistryRunPath, 0, KEY_ALL_ACCESS, &hTemp) == ERROR_SUCCESS)
    {
        if (bStartupRegistered()) RegDeleteValueA(hTemp, cConsole);
        else
        {
            char cCurrentFileName[MAX_PATH];
            GetModuleFileNameA(0, cCurrentFileName, MAX_PATH);
            RegSetValueExA(hTemp, cConsole, 0, REG_SZ, (unsigned char*)cCurrentFileName, strlen(cCurrentFileName));
        }
        RegCloseKey(hTemp);
    }
}
/**/

void SaveSettings() // clean output
{
    std::ofstream oFile("settings.json");
    if (oFile.is_open())
    {
        size_t sSize = nGlobal::vSettings.size();
        size_t sCount = 1;
        oFile << "{\n";
        oFile << "  \"Settings\":\n";
        oFile << "  [\n";
        if (sSize > 0)
        {
            for (CSettings* SettingsInfo : nGlobal::vSettings)
            {
                oFile << "    {\n";
                oFile << "        \"name\": \"" + SettingsInfo->sName + "\",\n";
                oFile << "        \"vibrance\": " + std::to_string(SettingsInfo->iVibrance) + "\n";
                if (sCount == sSize) oFile << "    }\n";
                else oFile << "    },\n";
                sCount++;
            }
        }
        oFile << "  ]\n";
        oFile << "}";
        oFile.close();
    }
}

void ClearConsole() // very big func
{
    system("cls"); // system func uh.
}

void CommandLine()
{
    bool bBreak = false;
    while (1)
    {
        ClearConsole();
        PrintfColor(RED, "[*] Go Back\n");
        PrintfColor(WHITE, "[0.] Toggle Startup [Status: ");
        if (bStartupRegistered()) PrintfColor(GREEN, "ON");
        else PrintfColor(RED, "OFF");
        PrintfColor(WHITE, "]\n[1.] Add Settings\n");
        PrintfColor(WHITE, "[2.] Remove Settings\n\n");
        PrintfColor(YELLOW, "Press key to choice.");
        switch (_getch())
        {
            case '0':
            {
                StartupRegisterToggle();
            }
            break;
            case '1':
            {
                ClearConsole();
                PrintfColor(WHITE, "[*] Add Settings:\n\n");

                CSettings* NewSettings = new CSettings;
                PrintfColor(YELLOW, "Process Name:"); PrintfColor(WHITE, " ");
                std::cin >> NewSettings->sName;
                PrintfColor(YELLOW, "Vibrance (-1 [OFF] | 0 - 50):"); PrintfColor(WHITE, " ");
                std::cin >> NewSettings->iVibrance;
                if (NewSettings->iVibrance != -1) NewSettings->iVibrance = std::clamp(NewSettings->iVibrance, 0, 50);
                nGlobal::vSettings.push_back(NewSettings);
                SaveSettings();
                MessageBoxA(0, std::string("Added new settings for: " + NewSettings->sName).c_str(), cConsole, MB_OK | MB_ICONASTERISK);
            }
            break;
            case '2':
            {
                ClearConsole();
                PrintfColor(WHITE, "[*] Remove Settings:\n\n");

                std::string sName;
                PrintfColor(YELLOW, "Process Name:"); PrintfColor(WHITE, " ");
                std::cin >> sName;
                bool bFound = false;
                int iCount = 0;
                for (CSettings* SettingsInfo : nGlobal::vSettings)
                {
                    if (SettingsInfo->sName.find(sName) != std::string::npos)
                    {
                        sName = SettingsInfo->sName;
                        bFound = true;
                        nGlobal::vSettings.erase(nGlobal::vSettings.begin() + iCount);
                        SaveSettings();
                        MessageBoxA(0, std::string("Removed settings for: " + sName).c_str(), cConsole, MB_OK | MB_ICONASTERISK);
                        break;
                    }
                    iCount++;
                }
                if (!bFound) MessageBoxA(0, std::string("Couldn't find settings with name: " + sName).c_str(), cConsole, MB_OK | MB_ICONERROR);
            }
            break;
            default:
            {
                bBreak = true;
            }
            break;
        }
        if (bBreak) break;
    }
}

void PrintCurrentSettings()
{
    ClearConsole();
    PrintfColor(WHITE, "[*] Loaded Settings:");
    if (nGlobal::vSettings.empty())  PrintfColor(RED, "\n\n[-] Settings are empty or broken json?");
    else
    {
        for (CSettings* SettingsInfo : nGlobal::vSettings)
        {
            PrintfColor(GREEN, "\n\n[+] ");
            PrintfColor(WHITE, "%s:\n", SettingsInfo->sName.c_str());
            if (SettingsInfo->iVibrance >= 0) PrintfColor(GREEN, "    [+] Vibrance: %i", SettingsInfo->iVibrance);
        }
    }

    PrintfColor(YELLOW, "\n\n[*] Press \'c\' to use command line.");
    if (_getch() == 'c') CommandLine();
}

void SetWorkingDirectory()
{
    char cCurrentFileName[MAX_PATH];
    GetModuleFileNameA(0, cCurrentFileName, MAX_PATH);
    std::string sTemp = cCurrentFileName;
    sTemp = sTemp.substr(0, sTemp.find_last_of("\\/"));
    SetCurrentDirectoryA(sTemp.c_str());
}

void __stdcall WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (dwEvent != EVENT_SYSTEM_FOREGROUND) return; // Probably not needed but just in case.

    static HWND hCurrentWindow = nullptr;
    if (hCurrentWindow == hwnd) return;
    hCurrentWindow = hwnd;

    CSettings TempSettings;
    if (!nGlobal::vSettings.empty())
    {
        static std::string sWindowName; sWindowName = HWND2EXE(hCurrentWindow);
        for (CSettings* SettingsInfo : nGlobal::vSettings)
        {
            if (sWindowName.find(SettingsInfo->sName) != std::string::npos)
            {
                TempSettings = *SettingsInfo;
                break;
            }
        }
    }
    SetDVCLevel(TempSettings.iVibrance);
}

void WorkingThread()
{
    RegisterHotKey(0, 1, MOD_ALT | MOD_NOREPEAT, VK_F2);
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, 0, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    MSG msg;
    while (GetMessageA(&msg, 0, 0, 0))
    {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) ToggleConsoleVisibility();
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

int main()
{
    CreateEventA(0, 0, 0, "NVAPIT00LS");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBoxA(0, "Application is already running!", cConsole, MB_OK | MB_ICONERROR);
        return 0;
    }
    SetWorkingDirectory();
    AllocConsole();
    SetConsoleTitleA(cConsole);
    nGlobal::hOutputConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    while (!nGlobal::hWindowConsole) nGlobal::hWindowConsole = FindWindowA(0, cConsole); // 0.01% Chance it fail just in case.
    ToggleConsoleVisibility();

    nGlobal::nAPI::hMod = LoadLibraryA("nvapi.dll");
    if (!nGlobal::nAPI::hMod)
    {
        MessageBoxA(0, "Couldn't load \"nvapi.dll\".", cConsole, MB_OK | MB_ICONERROR);
        return 0;
    }

    std::ifstream iFile("settings.json");
    if (iFile.is_open())
    {
        nlohmann::json jData;
        // Ugly try & catch when parsing fails.
        try 
        {
            jData = nlohmann::json::parse(iFile);
        }
        catch (...) 
        {
            // Yes...
        }
        if (!jData.is_discarded() && !jData["Settings"].empty())
        {
            int iCount = 0;
            for (auto iT = jData["Settings"].begin(); iT != jData["Settings"].end(); ++iT)
            {
                CSettings* NewSettings = new CSettings;
                GetSettings(NewSettings->sName, jData, iCount, "name");
                NewSettings->iVibrance = -1; GetSettings(NewSettings->iVibrance, jData, iCount, "vibrance");
                if (NewSettings->iVibrance != -1) NewSettings->iVibrance = std::clamp(NewSettings->iVibrance, 0, 50);
                nGlobal::vSettings.push_back(NewSettings);
                iCount++;
            }
        }
        iFile.close();
    }
   
    // Very bad codenz.
    nGlobal::nAPI::QueryInterface = tNVAPI_QueryInterface(GetProcAddress(nGlobal::nAPI::hMod, "nvapi_QueryInterface"));
    (*tNVAPI_EnumNvidiaDisplayHandle_t((*nGlobal::nAPI::QueryInterface)(0x9ABDD40D)))(0, &nGlobal::nAPI::iHandle); // Hardcoded monitor index 0, so better make loop.
    // nvapi.dll Sig: 55 8B EC 83 EC 14 A1 ? ? ? ? 3D ? ? ? ? 74 24 F6 40 1C 20 74 1E 80 78 19 04 72 18 68 ? ? ? ? 68 ? ? ? ? FF 70 14 FF 70 10 E8 ? ? ? ? 83 C4 10 6A 00 E8 ? ? ? ? 83 C4 04 84 C0 75 09 B8 ? ? ? ? 8B E5 5D C3 57
    // First: GetDVCInfo
    // Second SetDVCLevel
    // OP Codes: [30 5D 0D 10] -> [sub_address] (B4 09 24 17) -> (0x172409B4)
    nGlobal::nAPI::SetDVCLevel = tNVAPI_SetDVCLevel((*nGlobal::nAPI::QueryInterface)(0x172409B4));
    nGlobal::nAPI::nInfo.version = sizeof(NV_DISPLAY_DVC_INFO) | 0x10000;
    (*tNVAPI_GetDVCInfo((*nGlobal::nAPI::QueryInterface)(0x4085DE45)))(nGlobal::nAPI::iHandle, 0, &nGlobal::nAPI::nInfo);

    std::thread tWorkingThread(WorkingThread);
    while (1) PrintCurrentSettings();
    return 0;
}