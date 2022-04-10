// LWM2M_Final_CPP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
//#include <vector>
#include <string>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <iostream>
#include <thread>
#include <sstream>
#include <chrono>

#include "Utils.cpp"
#include "LWM2M_Client.h"
#include "PerformanceTimer.h"
#include "userInput.cpp"





#pragma comment(lib,"ws2_32.lib")

static bool isFinishedApp = false;
static bool applicationRunApp = true;


int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket);
void destroySocket(SOCKET& socket);

int main()
{
    LWM2M_Client client;
	std::thread userInterfaceThread(userInputLWM, std::ref(client), std::ref(isFinishedApp), std::ref(applicationRunApp));

	std::cout << "Hello World!\n";
    
    while (applicationRunApp);

    userInterfaceThread.join();
   
}


int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket)
{
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0) {
        std::cerr << "ERR" << wsResult << std::endl;
        return 0;
    }

    outSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (outSocket == INVALID_SOCKET) {
        std::cerr << "Cannot create socket #" << WSAGetLastError << std::endl;
        WSACleanup();
        return 0;
    }

    DWORD timeout = tout * 1000;
    setsockopt(outSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    int connResult = connect(outSocket, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Error " << WSAGetLastError << std::endl;
        destroySocket(outSocket);
        return 0;
    }
    return 1;
}

void destroySocket(SOCKET& socket)
{
    closesocket(socket);
    WSACleanup();
}
