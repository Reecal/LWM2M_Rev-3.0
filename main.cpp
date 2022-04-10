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

#include "Utils.h"
#include "LWM2M_Client.h"
#include "PerformanceTimer.h"
#include "userInput.cpp"





#pragma comment(lib,"ws2_32.lib")

static bool isFinishedApp = false;
static bool applicationRunApp = true;


/*int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket);
void destroySocket(SOCKET& socket);*/

int main()
{
    LWM2M_Client client;
	std::thread userInterfaceThread(userInputLWM, std::ref(client), std::ref(isFinishedApp), std::ref(applicationRunApp));

    SOCKET s;

    std::string ipAddr = "192.168.204.128";
    int port = 5683;
    const std::string epName = "C++_123";


    initializeSocket(ipAddr, port, 5, s);

	std::cout << "Socket Initialized\n";

    //send(s, "Hello", 5, 0);
    
    while (applicationRunApp);

    userInterfaceThread.join();
   
}
