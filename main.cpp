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

void changeReference(const char*& ptr, const char* text);
uint8_t rebootfunc(uint8_t d);
uint8_t send_fc(char* data, uint16_t data_len);
void application_timer(LWM2M_Client& client, bool& applicationRun);

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

    initializeSocket(ipAddr, port, 5, s);

    LOG_INFO("Socket Initialized...");

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
    std::thread timerThread(application_timer, std::ref(client), std::ref(applicationRunApp));
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
        /*ms_ctr++;
        if (ms_ctr >= 10)
        {
            client.advanceTime(1);
            ms_ctr = 0;
            num_ovf++;
        }
        //std::cout << ms_ctr << std::endl;
        Sleep(100);*/

        
    }

    userInterfaceThread.join();
    timerThread.join();
   
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

void application_timer(LWM2M_Client& client, bool& applicationRun)
{
	while (applicationRun)
	{
        Sleep(1000);
        client.advanceTime(1);
        //std::cout << "Tick" << std::endl;
	}
}