#include <iostream>
#include <windows.h> // Needed to use windows api calls
#include <Psapi.h>
#include <fstream>

using namespace std;

// need to be global so our keyboard hook can clean up
HANDLE hProc = NULL;
HWND Window = NULL;
HHOOK hHook = NULL;
HANDLE hThread = NULL;

int getModuleBase(HANDLE processHandle, const std::wstring& sModuleName)
{
    HMODULE* hModules = NULL;
    wchar_t szBuf[50];
    DWORD cModules;
    DWORD dwBase = -1;

    EnumProcessModules(processHandle, hModules, 0, &cModules);
    hModules = new HMODULE[cModules / sizeof(HMODULE)];

    if (EnumProcessModules(processHandle, hModules, cModules / sizeof(HMODULE), &cModules)) {
        for (size_t i = 0; i < cModules / sizeof(HMODULE); i++) {
            if (GetModuleBaseName(processHandle, hModules[i], szBuf, sizeof(szBuf))) {
                if (sModuleName.compare(szBuf) == 0) {
                    dwBase = (DWORD)hModules[i];
                    break;
                }
            }
        }
    }

    delete[] hModules;
    return dwBase;
}

DWORD_PTR GetProcessBaseAddress(DWORD processID)
{
    DWORD_PTR   baseAddress = 0;
    HANDLE      processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    HMODULE* moduleArray;
    LPBYTE      moduleArrayBytes;
    DWORD       bytesRequired;
    if (processHandle)
    {
        if (EnumProcessModules(processHandle, NULL, 0, &bytesRequired))
        {
            if (bytesRequired)
            {
                moduleArrayBytes = (LPBYTE)LocalAlloc(LPTR, bytesRequired);
                if (moduleArrayBytes)
                {
                    unsigned int moduleCount;
                    moduleCount = bytesRequired / sizeof(HMODULE);
                    moduleArray = (HMODULE*)moduleArrayBytes;
                    if (EnumProcessModules(processHandle, moduleArray, bytesRequired, &bytesRequired))
                    {
                        baseAddress = (DWORD_PTR)moduleArray[0];
                    }

                    LocalFree(moduleArrayBytes);
                }
            }
        }

        CloseHandle(processHandle);
    }

    return baseAddress;
}

int main()
{
    const char* findWord = "Hello";
    const char* anotherWord= "Ansty"; //New intended health Value
    DWORD procID;
    Window = FindWindow(NULL, L"ConsoleInterception");
    if (!GetWindowThreadProcessId(Window, &procID))
        return 0;
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (!hProc)
        return 0;
    const int nSize = 6;
    char lpBuffer[nSize]; //allocate mem
    const auto base = GetProcessBaseAddress(procID);
    if (base == 0)
        return 0;
    ofstream myfile;
    myfile.open("example.txt");
    SIZE_T lpNOR = 0;
    for (DWORD i = 0;; ++i)
    {
        if (ReadProcessMemory(hProc, (LPVOID)(base + i), lpBuffer, nSize, &lpNOR) == TRUE)
        {
            myfile << lpBuffer;
            if (strcmp(lpBuffer, findWord) == 0)
            {
                auto vmem = VirtualAllocEx(hProc, (LPVOID)(base + i), nSize + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                DWORD old_protection = PAGE_EXECUTE_READ;
                VirtualProtectEx(hProc, (LPVOID)(base + i), nSize, PAGE_EXECUTE_READWRITE, &old_protection);
                if (WriteProcessMemory(hProc, (LPVOID)(base + i), anotherWord, nSize, &lpNOR))
                    FlushInstructionCache(hProc, (LPVOID)(base + i), nSize);
                break;
            }
        }
    }
    myfile.close();
    return 1;
}