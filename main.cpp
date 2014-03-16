#include <QCoreApplication>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>

#define RBRPOINTER 0x007EAC48

DWORD getProcessByName(char * processName);
float floatFromBits( DWORD bits );



int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

    //        char* pApp = (char*) 0x007EAC48;
    //        char* app = (char*) *pApp;
    //        char* ffboffest = app + (0xBC0 + 0x3C + 0x08);


    DWORD rbrRealAddr = getProcessByName("RichardBurnsRally_SSE.exe");
    if (rbrRealAddr != 0) {
        printf("RBR found");
    } else {
        printf("norbrfound");
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, NULL, rbrRealAddr);
    if(!hProcess) {
        printf("Error OpenProcess %u\n",GetLastError());
    } else{
        DWORD pRBR = 0;
        DWORD rBuf = 0;
        DWORD pFFB = 0;

        BOOL rpmRet = ReadProcessMemory(hProcess,(LPCVOID)RBRPOINTER,&pRBR,4,&rBuf);
        if (!rpmRet) {
            printf("Error ReadProcessMemory %u\n",GetLastError());
        }

        pFFB = pRBR + (0xBC0 + 0x3C + 0x08);

        int i = 0;

        while(++i < 10000) {

            DWORD rFFBBuf = 0;

            BOOL ret = ReadProcessMemory(hProcess,(LPCVOID)RBRPOINTER,&pFFB,4,&rFFBBuf);
            if (!ret) {
                printf("Error ReadProcessMemory %u\n",GetLastError());
            }

            if (rFFBBuf == 4)
                printf("FFB: %d \n",floatFromBits(rBuf));
            else
                printf("error\n");

        }

        CloseHandle(hProcess);
    }

//    return a.exec();
}

float floatFromBits( DWORD bits )
{
    return *reinterpret_cast< float * >( &bits );
}

DWORD getProcessByName(char * processName)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if( !Process32First( hProcessSnap, &pe32 ) )
    {
        printf("Error getting Process\n");
        CloseHandle( hProcessSnap );
        return 0;
    }

    do
    {
        char str[50];
        std::wcstombs(str, pe32.szExeFile, 50);

        if(!_stricmp(str, processName))
        {
            CloseHandle(hProcessSnap);
            return pe32.th32ProcessID;
        }
    } while(Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return 0;
}
