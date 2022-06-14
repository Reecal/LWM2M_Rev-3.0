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
#include "userInput.h"
#include "LWM2M_Defines.h"
#include "Logger_xdvora2g.h"
#include "BG77.h"

//#define LOG_OUTPUT 1
//#define USE_BG77

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

//helper variables
static bool isFinishedApp = false;
static bool applicationRunApp = true;
extern bool main_loop_bool;

//functions definitions
uint8_t rebootfunc();
uint8_t send_fc(char* data, uint16_t data_len);
void application_timer(LWM2M_Client& client, bool& applicationRun);

//Initialize socket for Ethernet
SOCKET s;

//Initialize BG77 for NB-IoT and LTE Cat-M
BG77 bg(false);

//Initialize client
LWM2M_Client client("RD_EP", rebootfunc);

int main()
{
    srand(time(NULL));

	//Register autosend callback
    client.register_send_callback(send_fc);

    //Create some basic objects and resources
	client.createObject(3441, 0);
    client.addResource(3441, 0, 110, TYPE_STRING, READ_WRITE, false, (char*)"initial value");
    client.addResource(3441, 0, 120, TYPE_INT, READ_WRITE, false, (int) 1024);
    client.addResource(3441, 0, 130, TYPE_FLOAT, READ_WRITE, false, (float)3.14159);
    client.addResource(3441, 0, 140, TYPE_BOOLEAN, READ_WRITE, false, true);
    client.addResource(3441, 0, 1110, TYPE_STRING, READ_WRITE, true, (char*)"initial value");
    client.addResource(3441, 0, 1120, TYPE_INT, READ_WRITE, true, (int)1024);
    client.addResource(3441, 0, 1130, TYPE_FLOAT, READ_WRITE, true, (float)3.14159);
    client.addResource(3441, 0, 1140, TYPE_BOOLEAN, READ_WRITE, true, true);
    client.updateResource(3441, 0, 1110, "Hello", 1);


    client.createObject(4, 0);
#if defined(USE_BG77)
    client.addResource(4, 0, 0, TYPE_INT, READ_ONLY, false,(int) 7);
#else
    client.addResource(4, 0, 0, TYPE_INT, READ_ONLY, false, (int)41);
#endif
    client.addResource(4, 0, 1, TYPE_INT, READ_ONLY, true, (int)20);
    client.addResource(4, 0, 2, TYPE_INT, READ_ONLY, false, (int)-70);
    client.addResource(4, 0, 4, TYPE_STRING, READ_ONLY, false, (char*)"192.168.10.128");
    client.addResource(4, 0, 5, TYPE_STRING, READ_ONLY, false, (char*) "192.168.10.1");
    client.addResource(4, 0, 7, TYPE_STRING, READ_ONLY, false, (char*) "lpwa.vodafone.iot");
   
    client.createObject(3342, 0);
    client.addResource(3342, 0, 5500, TYPE_BOOLEAN, READ_ONLY, false, true);

    client.createObject(3328, 0);
    client.addResource(3328, 0, 5601, TYPE_FLOAT, READ_ONLY, false, (float)3.1452);


    






    LOG_INFO("Program start...");
    

    //std::string ipAddr = "192.168.10.142";
	std::string ipAddr = "192.168.204.128";
    int port = 5683;
    //std::string ipAddr = "62.245.65.221";
    //int port = 9431;
    const std::string epName = "C++_123";

    initializeSocket(ipAddr, port, 5, s);

    LOG_INFO("Socket Initialized...");

    main_loop_bool = true;
   
#if defined(USE_BG77)
    
    if (bg.checkCommunication() != BG77_SUCCESS)
    {
        std::cout << "BG77 not responding!\r\n";
        while (1);
    }
    bg.activatePDP(1);
    bg.openSocket(1, 9431);

#endif

    //Start respective threads
    //User input for debugging purposes
    std::thread userInterfaceThread(userInputLWM, std::ref(client), std::ref(isFinishedApp), std::ref(applicationRunApp));
    //Timer thread to provide timing to the library - simulates timer interrupts or freeRTOS scheduling
    std::thread timerThread(application_timer, std::ref(client), std::ref(applicationRunApp));

    while (applicationRunApp)
    {
        client.loop();
    	char outputBuffer[1500];

#if defined(USE_BG77)
        if (bg.readData(outputBuffer) == BG77_INCOMING_DATA)
        {
            Sleep(100);
            int len = 1;
            do
            {
                len = bg.pullData(outputBuffer);
                if (len > 0) client.receive(outputBuffer, len);
                Sleep(100);
            } while (len != 0);
        	
            
        }
        Sleep(10);
#else
        int num_bytes = recv(s, outputBuffer, 1500, 0);
        if (num_bytes > 0)
        {
            client.receive(outputBuffer, num_bytes);

        }
#endif

 
    }

    //Terminate the program
    userInterfaceThread.join();
    timerThread.join();
    client.client_deregister();
#if defined(USE_BG77)
    Sleep(1000);
    bg.closeSocket(1);
    bg.deactivatePDP(1);
#endif
    LOG_INFO("Exiting program...");
}


uint8_t rebootfunc()
{
    LOG_WARNING("Reboot: ");
    client.client_deregister();
    Sleep(5000);
    return 0;
}

uint8_t send_fc(char* data, uint16_t data_len)
{
    //Send data via Ethernet or BG77
#if defined(USE_BG77)
    bg.sendData(1, data, data_len, (char*)"62.245.65.221", 9431);
    return 0;
#else
    return send(s, data, data_len, 0);
#endif
    
}

void application_timer(LWM2M_Client& client, bool& applicationRun)
{
    //Move library time by 1s each second
	while (applicationRun)
	{
        Sleep(1000);
        client.advanceTime(1);
        //std::cout << "Tick" << std::endl;
	}
}