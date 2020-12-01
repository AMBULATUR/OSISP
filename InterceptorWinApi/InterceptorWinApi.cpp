#include <iostream>
#include <windows.h> // Needed to use windows api calls
#include <Psapi.h>
#include <fstream>
#define findWord "Hello"

using namespace std;

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
    HANDLE hProc = NULL;
    HWND Window = NULL;
    const char* Word = "FOUND"; //New intended health Value
    DWORD procID;
    Window = FindWindow(NULL, L"ConsoleInterception");
    if (!GetWindowThreadProcessId(Window, &procID))
        return 0;
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (!hProc)
        return 0;
    const int nSize = 6;
    char lpBuffer[nSize]; //allocate mem
    const auto PTRbase = GetProcessBaseAddress(procID);
    ofstream myfile;
    myfile.open("example.txt");
    SIZE_T lpNOR = 0;
    DWORD _offset = 0;
    while (true)
    {
        LPVOID OffSet = LPVOID(PTRbase + _offset++);
        if (ReadProcessMemory(hProc, OffSet, lpBuffer, nSize, &lpNOR) == TRUE)
        {
            myfile << lpBuffer;
            if (strcmp(lpBuffer, findWord) == 0)
            {
                DWORD flProtect = PAGE_EXECUTE_READWRITE;
                VirtualAllocEx(hProc, OffSet, nSize + 1, MEM_COMMIT | MEM_RESERVE,flProtect); // Выделяем память в другом процессе
                VirtualProtectEx(hProc, OffSet, nSize, PAGE_EXECUTE_READWRITE, &flProtect); //Change attribute
                /*
                    1й параметр - хэндл открытого процесса
                    2й - (указатель)адрес памяти, для которой необходимо изменить модификатор доступа
                    3й - размер блока памяти, для которого необходимо изменить модификатор доступа
                    4й - новый модификатор доступа
                    5й - (указатель)переменная размером 4 байта, в которую запишется старый модификатор доступа
                */WriteProcessMemory(hProc, OffSet, Word, nSize, &lpNOR);
                std::cout << "Success" << endl;
            }
        }
    }
    myfile.close();
    return 1;
}