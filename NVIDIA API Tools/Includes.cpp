#include "Includes.hpp"

namespace nGlobal
{
    namespace nAPI
    {
        HMODULE hMod;
        int iHandle;
        NV_DISPLAY_DVC_INFO nInfo;

        tNVAPI_QueryInterface QueryInterface;
        tNVAPI_SetDVCLevel SetDVCLevel;
    }

    HANDLE hOutputConsole;
    HWND hWindowConsole;
    std::vector<CSettings*> vSettings;
}