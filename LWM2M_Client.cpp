#include "LWM2M_Client.h"

#include <cstring>
#include <string.h>
#include "Logger_xdvora2g.h"



#define LOG_VERBOSITY 3

#define SET_TX_FLAG() (LWM2M_Client::flags |= 0x01)
#define CLEAR_TX_FLAG() (LWM2M_Client::flags &= ~0x01)

#define SET_RX_FLAG() (LWM2M_Client::flags |= 0x02)
#define CLEAR_RX_FLAG() (LWM2M_Client::flags &= ~0x02)

#define RX_FLAG		(LWM2M_Client::flags & 0x02)

#if LOG_OUTPUT == 1
#define LOG_ENTITY "\x1B[34mLWM2M_CLIENT\033[0m"
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


char* lw_buffer;
CoAP_Message_t last_message;

LWM2M_Client::LWM2M_Client(const char* ep_name, uint8_t(*reb)(uint8_t)) : endpoint_name(ep_name), reboot_cb(reb)
{
	flags = 0;
	rxBuffer_head = 0;
	rxBuffer_tail = 0;
	txBuffer_head = 0;
	txBuffer_tail = 0;
	sys_time = 0;
	lastUpdate = 0;
}



uint8_t LWM2M_Client::receive(char* data, uint16_t data_length)
{
	uint8_t newHeadIndex = (rxBuffer_head + 1) & (RX_BUFFER_MAX_SIZE-1);

	if (newHeadIndex == rxBuffer_tail)
	{
		return BUFFER_OVERFLOW;
	}

	data[data_length] = '\0';
	rxData[rxBuffer_head] = data;
	rxBuffer_head = newHeadIndex;
	SET_RX_FLAG();
	return 0;
}

LWM2M_Status LWM2M_Client::getStatus()
{
	return client_status;
}

uint8_t LWM2M_Client::getTxData(char*& outputBuffer)
{
	if (txBuffer_head == txBuffer_tail) return NO_DATA_AVAILABLE;

	outputBuffer = txData[txBuffer_tail];
	txBuffer_tail = (txBuffer_tail + 1) & (TX_BUFFER_MAX_SIZE - 1);

	if (txBuffer_head == txBuffer_tail)
	{
		CLEAR_TX_FLAG();
		if (client_status == REGISTERED_TX_DATA_READY)
		{
			client_status = REGISTERED_IDLE;
		}
	}
		

	if (client_status == REGISTRATION_SCHEDULED)
	{
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=4
		LOG_INFO("Registration request read from buffer...");
#endif
		client_status = AWAIT_REGISTRATION_RESPONSE;
	}
		

	return 0;

}

uint8_t LWM2M_Client::schedule_tx(char* data)
{
	uint8_t newHeadIndex = (txBuffer_head + 1) & (TX_BUFFER_MAX_SIZE-1);

	if (newHeadIndex == txBuffer_tail)
	{
		return BUFFER_OVERFLOW;
	}
	//client_status = REGISTERED_TX_DATA_READY;
	
	txData[txBuffer_head] = data;
	txBuffer_head = newHeadIndex;
	SET_TX_FLAG();
	client_status = REGISTERED_TX_DATA_READY;
	return 0;
}

uint8_t LWM2M_Client::getRxData(char*& outputBuffer)
{
	if (!RX_FLAG) return NO_DATA_AVAILABLE;

	char* ptr = rxData[rxBuffer_tail];
	//outputBuffer = rxData[rxBuffer_tail];
	outputBuffer = ptr;
	//strcpy_s(outputBuffer, 1000,rxData[rxBuffer_tail]);
	rxBuffer_tail = (rxBuffer_tail + 1) & (RX_BUFFER_MAX_SIZE - 1);

	if (rxBuffer_head == rxBuffer_tail) CLEAR_RX_FLAG();

	return 0;
}

void LWM2M_Client::register_send_callback(uint8_t(*send_func)(char* data, uint16_t data_len))
{
	send_cb = send_func;
}

uint8_t LWM2M_Client::send(char* data, uint16_t data_len)
{
	return send_cb(data, data_len);
}

uint8_t LWM2M_Client::send_registration()
{
	//temporary
	//TODO Write proper registration function
	/*char dataChar[] = "\x48\x02\x63\xc1\x08\x80\x6e\x17\x2e\xd0\x58\xe5\xb2rd\x11\x28\x33" \
		"b=U\x09lwm2m=1.0\x06lt=100\x0d\x06"\
		"ep=Radim_DP__LWM2M1\xff</>;ct=\"60 110 112 11542 11543\";rt=\"oma.lwm2m\",</1>;ver=1.1,</1/0>,</3>;ver=1.1,</3/0>,</6/0>,</3303>;ver=1.1,</3303/0>,</3441/0>";
		*/

	char payload[] = "</>;ct=\"60 11543\";rt=\"oma.lwm2m\",</1/0>,</3/0>,</6/0>,</3303>,</3303/0>,</3441/0>";

	CoAP_message_t coap_message;
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&coap_message, COAP_OPTIONS_CONTENT_FORMAT, APPLICATION_LINK_FORMAT);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "b=U");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lwm2m=1.0");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lt=" + to_string(lifetime));
	char epname[30];
	sprintf_s(epname, "ep=%s", endpoint_name);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, epname);

	CoAP_set_payload(&coap_message, payload);


	CoAP_assemble_message(&coap_message);

#if defined(AUTO_SEND)
	
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Sending registration request...");
#endif

	client_status = AWAIT_REGISTRATION_RESPONSE;
	//return send(dataChar, strlen(dataChar));
	return send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);
#else
	client_status = REGISTRATION_SCHEDULED;
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Scheduling registration request...");
#endif
	return schedule_tx(dataChar);
	
#endif


}

uint8_t LWM2M_Client::send_update()
{
	CoAP_message_t coap_message;
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, std::string(reg_id));
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lt=" + to_string(lifetime));
	CoAP_assemble_message(&coap_message);

#if defined(AUTO_SEND)

	#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Sending update...");
	#endif

	client_status = AWAIT_UPDATE_RESPONSE;
	//return send(dataChar, strlen(dataChar));
	return send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);
#else
	client_status = REGISTRATION_SCHEDULED;

	#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Scheduling update request...");
	#endif

	return schedule_tx(dataChar);

#endif

}

uint8_t LWM2M_Client::update_routine()
{
	if (client_status == NOT_REGISTERED || client_status == AWAIT_REGISTRATION_RESPONSE) return 1;
	if (client_status == AWAIT_UPDATE_RESPONSE)
	{
		//Try sending update again
		//std::cout << "RETRY: " << (lastUpdate + lifetime) << " : " << (lastUpdate + lifetime) % UPDATE_RETRY_TIMEOUT << std::endl;
		if (sys_time >= (lastUpdate + lifetime))
		{
			client_status = NOT_REGISTERED;

			#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_ERROR("Update response not received...");
			#endif
		}

		else if (((lastUpdate + lifetime) - sys_time) % UPDATE_RETRY_TIMEOUT == 0)
		{
			return send_update();
		}
	}
	else
	{
		if (sys_time >= (lastUpdate + lifetime - UPDATE_HYSTERESIS))
		{
			return send_update();
		}
	}

	

	return 0;
}

void LWM2M_Client::loop()
{
	//Check if the client is registered or not
	if (client_status == NOT_REGISTERED)
	{
#if defined(AUTO_REGISTER)
		send_registration();
#endif
		return;
	}

	update_routine();

	//Check buffer for incoming message
	if (getRxData(lw_buffer) == NO_DATA_AVAILABLE)
	{
		//If no message was received
		return;
	}

	//if message was received, try parsing it
	if (CoAP_parse_message(&last_message, (char*)lw_buffer, strlen(lw_buffer)) != COAP_OK)
	{
		//If there was a error in parsing the message or the message
		//		is not valid, drop the message and return
		return;
	}

	//The message should be parsed correctly here
	//print_message_info(&last_message);

	switch (client_status)
	{
	case AWAIT_REGISTRATION_RESPONSE:
		if (check_registration_message(&last_message) == REGISTRATION_SUCCESS)
		{
			save_registration_id(&last_message);
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_INFO("Registration successful...");
			LOG_INFO("Registration ID: \x1B[32m" + std::string(reg_id) + "\033[0m");
#endif
			client_status = REGISTERED_IDLE;
			lastUpdate = sys_time;
		}
		break;



	case AWAIT_UPDATE_RESPONSE:

		if (check_update_message(&last_message) == UPDATE_SUCCESS)
		{
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_INFO("Update successful...");
#endif
			client_status = REGISTERED_IDLE;
			lastUpdate = sys_time;
		}

		else
		{
			process_message(&last_message);
		}
		break;




	case REGISTERED_IDLE: case REGISTERED_TX_DATA_READY: case REGISTERED_AWAIT_RESPONSE: default:
		process_message(&last_message);
		break;
	}

}

void LWM2M_Client::advanceTime(uint16_t amount_in_seconds)
{
	sys_time += amount_in_seconds;
}

uint8_t LWM2M_Client::check_registration_message(CoAP_message_t* c)
{
	if (c->header.type == COAP_ACK && c->header.returnCode == COAP_SUCCESS_CREATED)
	{
		//TODO: Properly check registration message
		//if (strcmp((const char*)(c->options.options[0].option_value), "rd")) std::cout << c->options.options[0].option_value << std::endl;



		//temporary
		std::string loc_path = CoAP_get_option_string(c, COAP_OPTIONS_LOCATION_PATH);
		if (loc_path.substr(0,2) == "rd") return REGISTRATION_SUCCESS;
		
	}
	return REGISTRATION_FAILED;
	
}

uint8_t LWM2M_Client::check_update_message(CoAP_message_t* c)
{
	if (c->header.type == COAP_ACK && c->header.returnCode == COAP_SUCCESS_CHANGED)
	{
		return UPDATE_SUCCESS;
	}
	return UPDATE_FAILED;

}

void LWM2M_Client::save_registration_id(CoAP_message_t* c)
{
	//TODO: Parse it properly from URI
	for (uint8_t i = 0; i < c->options.options[1].option_length; i++)
	{
		reg_id[i] = c->options.options[1].option_value[i];
	}
	reg_id[c->options.options[1].option_length] = 0;
}

uint8_t LWM2M_Client::process_message(CoAP_message_t* c)
{
	print_message_info(c);
	char uri_buffer[50];
	CoAP_get_option_chars(c, COAP_OPTIONS_LOCATION_PATH, uri_buffer);





	if(c->header.type != COAP_ACK)
	{
		CoAP_message_t error_message;
		int message_type = c->header.type == COAP_CON ? COAP_ACK : COAP_NON;
		CoAP_tx_setup(&error_message, message_type, c->header.token_length, COAP_C_ERR_NOT_FOUND, c->header.messageID, c->header.token);
		CoAP_set_payload(&error_message, "Not found!");
		CoAP_assemble_message(&error_message);
		send((char*)(error_message.raw_data.masg.data()), error_message.raw_data.message_total);
	}
	

	return 0;
}

void LWM2M_Client::print_message_info(CoAP_message_t* c)
{
	char outbuffer[50];
	CoAP_get_option_chars(c, COAP_OPTIONS_LOCATION_PATH, outbuffer);
	std::cout << "LOCATION PATH: " << outbuffer << std::endl;
	CoAP_get_option_chars(c, COAP_OPTIONS_URI_PATH, outbuffer);
	std::cout << "URI_PATH: " << outbuffer << std::endl;
	CoAP_get_option_chars(c, COAP_OPTIONS_URI_QUERY, outbuffer);
	std::cout << "URI_QUERY: " << outbuffer << std::endl;
	CoAP_get_option_chars(c, COAP_OPTIONS_OBSERVE, outbuffer);
	std::cout << "OBSERVE: " << outbuffer << std::endl;
	
	
	


	/*std::cout << "LOCATION PATH: " << CoAP_get_option_string(c, COAP_OPTIONS_LOCATION_PATH) << std::endl;
	std::cout << "URI_PATH: " << CoAP_get_option_string(c, COAP_OPTIONS_URI_PATH) << std::endl;
	std::cout << "URI_QUERY: " << CoAP_get_option_string(c, COAP_OPTIONS_URI_QUERY) << std::endl;
	std::cout << "OBSERVE: " << CoAP_get_option_string(c, COAP_OPTIONS_OBSERVE) << std::endl;*/
}

uint8_t LWM2M_Client::client_deregister() {
	//cout << "Client deregister" << endl;
	//cout << LWM2M_Client::registration_status << endl;
	if (client_status == NOT_REGISTERED) {
		LOG_WARNING("client_deregister: Client is not registered to any server");
		//cout << "Client is not registered to any server" << endl;
		return 1;
	}
	CoAP_message_t deregister_message;
	CoAP_tx_setup(&deregister_message, COAP_CON, 8, COAP_METHOD_DELETE);
	CoAP_add_option(&deregister_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&deregister_message, COAP_OPTIONS_URI_PATH, std::string(reg_id));
	CoAP_assemble_message(&deregister_message);
	send((char*)(deregister_message.raw_data.masg.data()), deregister_message.raw_data.message_total);
	client_status = NOT_REGISTERED;
	LOG_INFO("Deregistration message sent.");
	return 0;
}