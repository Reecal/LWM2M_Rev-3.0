#pragma once

#include <cstdint>
#include "LWM2M_Defines.h"

class LWM2M_Client
{

	
	LWM2M_Status client_status = NOT_REGISTERED;
	uint8_t flags = 0;

	uint8_t rxBuffer_head = 0;
	uint8_t rxBuffer_tail = 0;
	uint8_t txBuffer_head = 0;
	uint8_t txBuffer_tail = 0;

	char* txData[TX_BUFFER_MAX_SIZE] = {0};
	char* rxData[RX_BUFFER_MAX_SIZE] = {0};

	uint8_t (*reboot_cb)(uint8_t);
	const char* endpoint_name;


public:
	LWM2M_Client(const char* ep_name, uint8_t(*reb)(uint8_t));
	uint8_t receive(char* data, uint16_t data_length);
	LWM2M_Status getStatus();
	uint8_t getTxData(char*& outputBuffer);
	uint8_t schedule_tx(char* data);

	void loop();


};