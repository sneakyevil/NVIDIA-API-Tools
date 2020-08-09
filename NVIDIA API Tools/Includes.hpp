#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <thread>
#include "json.hpp"

template <class cType>
void GetSettings(cType &cValue, nlohmann::json jData, int iCount, std::string sKey)
{
    if (!jData["Settings"][iCount][sKey].is_null()) cValue = jData["Settings"][iCount][sKey].get<cType>();
}

typedef struct
{
    unsigned int version;
    int currentLevel;
    int minLevel;
    int maxLevel;
} NV_DISPLAY_DVC_INFO;

typedef int* (*tNVAPI_QueryInterface)(unsigned int offset);
typedef int (*tNVAPI_EnumNvidiaDisplayHandle_t)(int thisEnum, int* pNvDispHandle);
typedef int (*tNVAPI_GetDVCInfo)(int hNvDisplay, int outputId, NV_DISPLAY_DVC_INFO* pDVCInfo);
typedef int (*tNVAPI_SetDVCLevel)(int handle, int outputId, int level);

class CSettings
{
public:
    std::string sName;
    int iVibrance;
};

namespace nGlobal
{
    namespace nAPI
    {
        extern HMODULE hMod;
        extern int iHandle;
        extern NV_DISPLAY_DVC_INFO nInfo;

        extern tNVAPI_QueryInterface QueryInterface;
        extern tNVAPI_SetDVCLevel SetDVCLevel;
    }

    extern HANDLE hOutputConsole;
    extern HWND hWindowConsole;
    extern std::vector<CSettings*> vSettings;
}