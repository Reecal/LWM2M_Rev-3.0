#pragma once

#include <cstdint>
#include "LWM2M_Defines.h"
#include "CoAP.hpp"

class LWM2M_Client
{

	struct Data_In_t
	{
		char* data;
		uint16_t data_length;
	};


private:
	LWM2M_Status client_status = NOT_REGISTERED;
	uint8_t flags = 0;

	char* txData[TX_BUFFER_MAX_SIZE] = { 0 };
	char* rxData[RX_BUFFER_MAX_SIZE] = { 0 };

	uint8_t rxBuffer_head = 0;
	uint8_t rxBuffer_tail = 0;
	uint8_t txBuffer_head = 0;
	uint8_t txBuffer_tail = 0;

	
	//temp
	uint16_t lifetime = 30;

	long long lastUpdate;
	long long sys_time = 0;

	
	
	const char* endpoint_name;
	char reg_id[12];

	uint8_t(*reboot_cb)(uint8_t);
	uint8_t(*send_cb)(char* data, uint16_t data_len);

	uint8_t schedule_tx(char* data);
	uint8_t getRxData(char*& outputBuffer);
	uint8_t send_registration();
	uint8_t send_update();
	uint8_t update_routine();
	uint8_t check_registration_message(CoAP_message_t* c);
	uint8_t check_update_message(CoAP_message_t* c);
	void save_registration_id(CoAP_message_t* c);
	uint8_t process_message(CoAP_message_t* c);

public:
	LWM2M_Client(const char* ep_name, uint8_t(*reb)(uint8_t));
	uint8_t receive(char* data, uint16_t data_length);
	LWM2M_Status getStatus();
	uint8_t getTxData(char*& outputBuffer);
	void advanceTime(uint16_t amount_in_seconds);
	

	void register_send_callback(uint8_t(*send_func)(char* data, uint16_t data_len));

	//temp
	uint8_t send(char* data, uint16_t data_len);

	void loop();


};