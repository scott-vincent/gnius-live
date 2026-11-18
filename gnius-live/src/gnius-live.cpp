#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <thread>
#include "gnius-live.h"

const char* versionString = "v1.0.0";

bool _quit;

void gniusServer();


void milliSleep(int milliSecs)
{
#ifdef _WINDOWS
    Sleep(milliSecs);
#else
    usleep(milliSecs * 1000);
#endif
}

void globalInit()
{
#ifdef _WINDOWS
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) {
        printf("Failed to initialise Windows Sockets: %d\n", err);
        return;
    }
#endif
}

void globalCleanup()
{
#ifdef _WINDOWS
    WSACleanup();
#endif
}

int main(int argc, char** argv)
{
    globalInit();

    printf("Gnius-Live %s Copyright (c) 2026 Scott Vincent\n", versionString);
    fflush(stdout);

    gniusServer();

    globalCleanup();
    return 0;
}
