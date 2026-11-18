#ifdef _WINDOWS
#include <windows.h>
typedef int socklen_t;
#else
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKADDR sockaddr
#define closesocket close
#endif
#include <iostream>
#include "gnius-live.h"

extern bool _quit;

char teamspeakHost[32];
char data[MaxData];

static bool _initialised = false;
static SOCKET sockfd;
static sockaddr_in senderAddr;
static int addrSize = sizeof(senderAddr);
static int bytes;

void milliSleep(int milliSecs);


static void serverInit()
{
    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        printf("Server failed to create UDP socket\n");
        fflush(stdout);
        return;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(GniusLivePort);

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WINDOWS
        int error = WSAGetLastError();
#else
        int error = errno;
#endif
        printf("Server failed to bind to localhost port %d: %ld\n", GniusLivePort, error);
        fflush(stdout);
        return;
    }

    printf("Server listening on port %d\n", GniusLivePort);
    fflush(stdout);

    _initialised = true;
}

static void serverCleanUp()
{
    closesocket(sockfd);

    printf("G-NIUS Server exiting\n");
    _quit = true;
}

void readTeamspeakHost()
{
    FILE* inf = fopen(TeamspeakHostFile, "r");
    if (!inf) {
        printf("Failed to read file: %s\n", TeamspeakHostFile);
        *teamspeakHost = '\0';
        return;
    }

    int size = fread(teamspeakHost, 1, 32, inf);
    teamspeakHost[size] = '\0';

    fclose(inf);

    printf("Got Teamspeak IP address: %s\n", teamspeakHost);
}

void writeTeamspeakHost()
{
    printf("Got Teamspeak IP address: %s\n", teamspeakHost);

    FILE* outf = fopen(TeamspeakHostFile, "w");
    if (!outf) {
        printf("Failed to write file: %s\n", TeamspeakHostFile);
        return;
    }
    fprintf(outf, "%s\n", teamspeakHost);
    fclose(outf);
}

void forwardData()
{
    printf("Forward data %s\n", data);
    fflush(stdout);

    // Forward to able-live

    // Forward to RT host
    if (*teamspeakHost == '\0') {
        return;
    }


}

void gniusServer()
{
    serverInit();
    if (!_initialised) {
        serverCleanUp();
        return;
    }

    timeval timeout;

    while (!_quit) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);

        // Timeout needs to be set every time on Linux!
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        // Wait for Simulator (or radiotelephony) to send data (non-blocking, 2 second timeout)
        int sel = select(FD_SETSIZE, &fds, 0, 0, &timeout);
        if (sel > 0) {
            bytes = recvfrom(sockfd, data, MaxData, 0, (SOCKADDR*)&senderAddr, (socklen_t*)&addrSize);
            if (bytes == -1) {
#ifdef _WINDOWS
                int error = WSAGetLastError();
#else
                int error = errno;
#endif
                if (error == 10040) {
                    printf("Received more than %d bytes from %s (WSAError = %d)\n", MaxData, inet_ntoa(senderAddr.sin_addr), error);
                }
                else {
                    printf("Received from %s but WSAError = %d\n", inet_ntoa(senderAddr.sin_addr), error);
                }
                fflush(stdout);
            }
            else {
                data[bytes] = '\0';
                if (bytes > 4 && strncmp(data, "IP:", 3) == 0) {
                    strcpy(teamspeakHost, &data[3]);
                    writeTeamspeakHost();
                }
                else {
                    forwardData();
                }
            }
        }
        else {
            bytes = SOCKET_ERROR;
        }

        if (bytes == SOCKET_ERROR) {
            milliSleep(500);
        }
    }

    serverCleanUp();
}
