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
#include "LWM2M_Defines.h"
#include "Logger_xdvora2g.h"

//#define LOG_OUTPUT 1

#if LOG_OUTPUT == 1
//#define LOG_ENTITY "\x1B[34mMain\033[0m"
#define LOG_ENTITY "\x1B[34mMain\033[0m"
#define LOG_DATA(x,y)   LOG123(x, std::string(LOG_ENTITY), std::string(y))
#define LOG_INFO(x)     LOG123(LOG_INFO_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#define LOG_WARNING(x)  LOG123(LOG_WARNING_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#define LOG_ERROR(x)    LOG123(LOG_ERROR_MESSAGE_TYPE, std::string(LOG_ENTITY), std::string(x))
#else
#define LOG_DATA(x, y) 
#define LOG_INFO(x)
#define LOG_WARNING(x)
#define LOG_ERROR(x) 
#endif


#pragma comment(lib,"ws2_32.lib")

static bool isFinishedApp = false;
static bool applicationRunApp = true;
extern bool main_loop_bool;


/*int initializeSocket(std::string ipAddress, int port, int tout, SOCKET& outSocket);
void destroySocket(SOCKET& socket);*/
void changeReference(const char*& ptr, const char* text);
uint8_t rebootfunc(uint8_t d);
uint8_t send_fc(char* data, uint16_t data_len);

SOCKET s;

int main()
{
    srand(time(NULL));

	LWM2M_Client client("RD_EP", rebootfunc);
    client.register_send_callback(send_fc);
	

    LOG_INFO("Program start...");
    

    //std::string ipAddr = "192.168.10.142";
	std::string ipAddr = "192.168.204.128";
    int port = 5683;
    const std::string epName = "C++_123";

    std::string dat = "\x48\x02\x63\xc1\x08\x80\x6e\x17\x2e\xd0\x58\xe5\xb2rd\x11\x28\x33" \
        "b=U\x09lwm2m=1.0\x06lt=100\x0d\x06"\
        "ep=Radim_DP__LWM2M1\xff</>;ct=\"60 110 112 11542 11543\";rt=\"oma.lwm2m\",</1>;ver=1.1,</1/0>,</3>;ver=1.1,</3/0>,</6/0>,</3303>;ver=1.1,</3303/0>,</3441/0>";

    char dataChar[] = "\x48\x02\x63\xc1\x08\x80\x6e\x17\x2e\xd0\x58\xe5\xb2rd\x11\x28\x33" \
        "b=U\x09lwm2m=1.0\x06lt=100\x0d\x06"\
        "ep=Radim_DP__LWM2M1\xff</>;ct=\"60 110 112 11542 11543\";rt=\"oma.lwm2m\",</1>;ver=1.1,</1/0>,</3>;ver=1.1,</3/0>,</6/0>,</3303>;ver=1.1,</3303/0>,</3441/0>";



    initializeSocket(ipAddr, port, 5, s);

    LOG_INFO("Socket Initialized...");
	//std::cout << "Socket Initialized\n" << dat.length() << std::endl;

    /*const char* Ss = "Hello world!";
    const char* Ll = "Notify me!";

    changeReference(Ss, Ll);

    changeReference(Ss, "Yuhu");

    std::cout << Ss << " " << Ll << std::endl;*/

    //client.schedule_tx(dataChar);

    /*if (client.getStatus() == NOT_REGISTERED)
    {
        char* output;
        uint8_t response = client.getTxData(output);
        //send(s, output, strlen(output), 0);
        client.send(output, strlen(output));
       // send(s, , dat.length(), 0);
    }*/
       
    int ms_ctr = 0;
    int num_ovf = 0;
    main_loop_bool = true;
    std::thread userInterfaceThread(userInputLWM, std::ref(client), std::ref(isFinishedApp), std::ref(applicationRunApp));
    while (applicationRunApp)
    {
        client.loop();
    	char outputBuffer[1500];

        int num_bytes = recv(s, outputBuffer, 1500, 0);
        if (num_bytes > 0)
        {
            client.receive(outputBuffer, num_bytes);
            
        }
        //client.loop();
        ms_ctr++;
        if (ms_ctr >= 10)
        {
            client.advanceTime(1);
            ms_ctr = 0;
            num_ovf++;
        }
        Sleep(100);

        
    }

    userInterfaceThread.join();
   
}

void changeReference(const char*& ptr, const char* text)
{
    ptr = text;
}

uint8_t rebootfunc(uint8_t d)
{
    std::cout << "Reboot " << d << std::endl;
    return 0;
}

uint8_t send_fc(char* data, uint16_t data_len)
{
    
    return send(s, data, data_len, 0);
}