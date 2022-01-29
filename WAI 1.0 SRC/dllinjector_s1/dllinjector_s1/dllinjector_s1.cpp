#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>

using namespace std;

DWORD GetProcId(const char* procName) // 그저 프로세스 ID를 얻기 위한 함수
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_stricmp(procEntry.szExeFile, procName))
                {
                    procId = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

int main()
{
    // fstream.
    string fName("dll.txt");
    string LOCATION;
    ifstream iFile(fName);

    if (!iFile.is_open()) {
        return -1;
    }
    else {
        
        // seek ptr to end,
        iFile.seekg(0, std::ios::end);

        // and get txt's file size.
        int txtSz = iFile.tellg();

        // and resize the string var to txtSz,
        LOCATION.resize(txtSz);

        // seek ptr to beginning.
        iFile.seekg(0, std::ios::beg);

        // read.
        iFile.read(&LOCATION[0], txtSz);

    }

    const char* dllPath = LOCATION.c_str();
    const char* procName = "ac_client.exe";
    DWORD procId = 0;

    while (!procId)
    {
        procId = GetProcId(procName);
        Sleep(30);
    }

    std::cout << "Got Process ID: " << procId << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // "ac_client"의 Virtual Memory에서 dllPath를 적기 위한 메모리를 빌립니다. (하지만 loc이 0이 될 수는 없습니다.)

        std::cout << "Allocated memory for writing path: " << loc << std::endl;

        if (loc) { // loc이 0이 될 만약의 가능성을 제시해 이를 금지합니다.
            WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0); // loc에 dllPath를 적습니다.
        }

        char for_check[MAX_PATH];
        if (loc) {
            ReadProcessMemory(hProc, loc, &for_check, strlen(dllPath) + 1, 0);
        }

        std::cout << "Writed into process memory: " << for_check << ", " << loc << std::endl;

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0); // "ac_client.exe"에 loc에 저장되어있는 dllPath를 LoadLibraryA 함수를 통해서 부릅니다. 이로서 Dll Injecting은 완료되었습니다. 이제 이 dll injector를 닫습니다.

        std::cout << "Waiting until the thread starts...\nIf it waits all day, check your dll.txt file." << std::endl;

        if (hThread)
        {
            WaitForSingleObject(hThread, INFINITE);
            std::cout << "Success!" << std::endl;
            CloseHandle(hThread);
        }
        else {
            std::cerr << "Something went wrong." << std::endl;
            return -1;
        }

    }

    if (hProc)
    {
        CloseHandle(hProc);
    }
    return 0;
}