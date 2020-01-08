// Booster.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include "..\PriorityBooster\PriorityBoosterCommon.h"


int main(int argc, const char* argv[])
{
    if(argc < 3) 
    {
        printf("Usage: Booster <threadid> <priority>\n");
        return 0;
    }

    /*
    *       Abrir handle al dispositivo del driver
    */

    HANDLE hDevice = CreateFile(L"\\\\.\\PriorityBooster", GENERIC_WRITE,
        FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("%s (error=%d)\n", "Failed to open device", GetLastError());
    }

    /*
    *    Añadir a ThreadData el PID del proceso y el nivel de prioridad que se desea entre 1-31
    */
    ThreadData data;
    data.ThreadId = atoi(argv[1]); // command line first argument
    data.Priority = atoi(argv[2]); // command line second argument


    /*
    *   Mediante DeviceIoControl enviar el codigo de control IOCTL_PRIORITY_BOOSTER_SET_PRIORITY y enviar el buffer ThreadData data
    */
    DWORD returned;
    BOOL success = DeviceIoControl(hDevice,
        IOCTL_PRIORITY_BOOSTER_SET_PRIORITY,    // control code
        &data, sizeof(data),                    // input buffer and length
        nullptr, 0,                             // output buffer and length
        &returned, nullptr);

    if (success)
    {
        printf("Priority change succeeded!\n");
    }
    else
    {
        printf("%s (error=%d)\n", "Priority change failed!", GetLastError());
    }

    CloseHandle(hDevice);
}

