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

    std::string dat = "\x48\x02\x63\xc1\x08\x80\x6e\x17\x2e\xd0\x58\xe5\xb2rd\x11\x28\x33" \
        "b=U\x09lwm2m=1.0\x06lt=100\x0d\x06"\
        "ep=Reecal44CZESP-32\xff</>;ct=\"60 110 112 11542 11543\";rt=\"oma.lwm2m\",</1>;ver=1.1,</1/0>,</3>;ver=1.1,</3/0>,</6/0>,</3303>;ver=1.1,</3303/0>,</3441/0>";


    initializeSocket(ipAddr, port, 5, s);

	std::cout << "Socket Initialized\n";

    send(s, dat.c_str(), dat.length(), 0);
    
    while (applicationRunApp);

    userInterfaceThread.join();
   
}
