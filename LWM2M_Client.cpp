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

uint8_t update_already_sent = 0;
long long last_update_sent = 0;

char* lw_buffer;
CoAP_Message_t last_message;

uint8_t(*send_update_cb)();

/**
 *	Default constructor for the Client class
 *
 *	INPUT  : ep_name - Endpoint name - Name of the device
 *			 reb - Pointer to a reboot function
 *
 */
LWM2M_Client::LWM2M_Client(const char* ep_name, uint8_t(*reb)()) : endpoint_name(ep_name), reboot_cb(reb)
{
	//Create mandatory objects
	LWM2M_Object obj1(1);
	obj1.add_resource(0, TYPE_INT, READ_ONLY, false, (int) 25);
	obj1.add_resource(1, TYPE_INT, READ_WRITE, false, (int)600);
	obj1.add_resource(6, TYPE_BOOLEAN, READ_WRITE, false, true);
	obj1.add_resource(7, TYPE_STRING, READ_WRITE, false, (char*) "U");
	obj1.add_executable_resource(8);
#if defined(USE_VECTORS)
	objects_vector.push_back(obj1);
#else
	object_ids[next_obj_ptr] = 1;
	objects[next_obj_ptr] = obj1;
	next_obj_ptr++;
#endif



	LWM2M_Object obj3(3);
	//obj3.add_resource(0, TYPE_STRING, READ_ONLY, false, (char*)"Diploma Thesis Radim Dvorak");
	obj3.add_resource(0, TYPE_STRING, READ_ONLY, false, (char*)"DP Thesis RD");
	obj3.add_resource(3, TYPE_STRING, READ_ONLY, false, (char*)"Rev 3.0");
	obj3.add_executable_resource(4, reboot_cb);
	obj3.add_resource(11, TYPE_INT, READ_ONLY, true, 0);
	obj3.add_resource(16, TYPE_STRING, READ_ONLY, false, (char*) "U");
	
#if defined(USE_VECTORS)
	objects_vector.push_back(obj3);
#else
	object_ids[next_obj_ptr] = 3;
	objects[next_obj_ptr] = obj3;
	next_obj_ptr++;
#endif
	srand(68);
	last_mid = rand() % 65536;
}


/**
 *	Main function to process incoming message.
 *
 *	INPUT  : data - Data to be copied into receive buffer
 *			 data_length - length of data
 *
 *	RETURN : Success or failure (Buffer Overflow)
 *
 */
uint8_t LWM2M_Client::receive(char* data, uint16_t data_length)
{
	uint8_t newHeadIndex = (rxBuffer_head + 1) & (RX_BUFFER_MAX_SIZE-1);

	if (newHeadIndex == rxBuffer_tail)
	{
		return BUFFER_OVERFLOW;
	}

	msg_lengths[rxBuffer_head] = data_length;
	data[data_length] = '\0';
	rxData[rxBuffer_head] = data;
	rxBuffer_head = newHeadIndex;
	SET_RX_FLAG();
	return 0;
}

/**
 *	Query for the current status of the client.
 *
 *	RETURN : Client status
 *
 */
LWM2M_Status LWM2M_Client::getStatus()
{
	return client_status;
}

/**
 *	Copies data from the TX buffer to the outputBuffer. If it is the last message, clears TX flag.
 *
 *	INPUT  : outputBuffer - output buffer to which data will be copied
 *	RETURN : Success or failure (Buffer overflow)
 *
 */
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

/**
*	Schedules TX of data to the TX buffer and sets corresponding flags
*	
*	INPUT : data - data to be sent
*
*	RETURN : Success or Error code
*
*/
uint8_t LWM2M_Client::schedule_tx(char* data)
{
	uint8_t newHeadIndex = (txBuffer_head + 1) & (TX_BUFFER_MAX_SIZE-1);

	if (newHeadIndex == txBuffer_tail)
	{
		return BUFFER_OVERFLOW;
	}
	
	txData[txBuffer_head] = data;
	txBuffer_head = newHeadIndex;
	SET_TX_FLAG();
	client_status = REGISTERED_TX_DATA_READY;
	return 0;
}

/**
 *	Reads incoming data
 *
 *	INPUT : outputBuffer - buffer in which read data will be copied to
 *
 *	RETURN : Data available or No data available and their respective codes.
 *
 */
uint16_t LWM2M_Client::getRxData(char*& outputBuffer)
{
	if (!RX_FLAG) return NO_DATA_AVAILABLE;

	char* ptr = rxData[rxBuffer_tail];
	uint16_t msgLen = msg_lengths[rxBuffer_tail];
	outputBuffer = ptr;
	rxBuffer_tail = (rxBuffer_tail + 1) & (RX_BUFFER_MAX_SIZE - 1);

	if (rxBuffer_head == rxBuffer_tail) CLEAR_RX_FLAG();

	return msgLen;
}


/**
 *	Registers and saved pointer to a function used to send data
 *
 *	INPUT : send_func - Pointer to a send function. Has to have two inputs. Data and data_length
 *
 *
 *	RETURN : Value saved in a resource in a string format
 */
void LWM2M_Client::register_send_callback(uint8_t(*send_func)(char* data, uint16_t data_len))
{
	send_cb = send_func;
}

uint8_t LWM2M_Client::send(char* data, uint16_t data_len)
{
	return send_cb(data, data_len);
}

/**
 *	Directly sends or schedules a TX with a registration message to the server
 *
 *	RETURN : Success or Error code
 *
 */
uint8_t LWM2M_Client::send_registration()
{
	//Assemble link format data according to the specification.
	std::string pl = "</>;ct=";

	//Add supported formats to the message
#if MULTI_VALUE_FORMAT == SINGLE_VALUE_FORMAT
	pl += std::to_string(MULTI_VALUE_FORMAT);
#else
	pl += "\"" + std::to_string(SINGLE_VALUE_FORMAT) + " " + std::to_string(MULTI_VALUE_FORMAT) + "\"";
#endif

	//Add version
	pl += ";rt=\"oma.lwm2m\",";

	//Add supported objects and instances to the message
#if defined(USE_VECTORS)
	uint8_t ctr = 0;
	for(LWM2M_Object& o : objects_vector)
	{
		ctr++;
		pl += "</";
		pl += to_string(o.getObject_id()) + "/" + std::to_string(o.getInstance_id());
		pl += ">";
		if (ctr != objects_vector.size()) pl += ",";
	}
#else
	for(uint8_t i = 0; i < next_obj_ptr; i++)
	{
		pl += "</";
		pl += to_string(objects[i].getObject_id()) + "/" + std::to_string(objects[i].getInstance_id());
		pl += ">";
		if (i != next_obj_ptr - 1) pl += ",";
	}
#endif

	//Assemble registration message
	CoAP_message_t coap_message;
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST, last_mid++);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&coap_message, COAP_OPTIONS_CONTENT_FORMAT, APPLICATION_LINK_FORMAT);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "b=" + getObject(1).getResource(7).getValue());
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lwm2m=1.0");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lt=" + getObject(1).getResource(1).getValue());
	char epname[30];
	sprintf_s(epname, "ep=%s", endpoint_name);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, epname);

	CoAP_set_payload(&coap_message, pl);


	CoAP_assemble_message(&coap_message);

#if defined(AUTO_SEND)
	
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Sending registration request...");
#endif

	client_status = AWAIT_REGISTRATION_RESPONSE;
	//return send(dataChar, strlen(dataChar));
	last_update_sent = sys_time;
	return send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);
#else
	client_status = REGISTRATION_SCHEDULED;
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Scheduling registration request...");
#endif
	return schedule_tx(dataChar);
	
#endif


}


/**
 *	Directly sends or schedules update
 *
 *	RETURN : Success or failure
 *
 */
uint8_t LWM2M_Client::send_update()
{
	//Assemble update message
	CoAP_message_t coap_message;
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST, last_mid++);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, std::string(reg_id));
	if (getObject(1).getResource(1).getValueChanged() == true)
	{
		CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lt=" + getObject(1).getResource(1).getValue());
		getObject(1).getResource(1).clearValueChanged();
	}
	CoAP_assemble_message(&coap_message);

#if defined(AUTO_SEND)

	#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Sending update...");
	#endif

	client_status = AWAIT_UPDATE_RESPONSE;
	//return send(dataChar, strlen(dataChar));
	last_update_sent = sys_time;
	return send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);
#else
	client_status = REGISTRATION_SCHEDULED;

	#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=3
	LOG_INFO("Scheduling update request...");
	#endif

	return schedule_tx(dataChar);

#endif

}


/**
 *	Checks last update or registration timings and decides whether an update should be sent
 *
 *	RETURN : Update sent or not
 *
 */
uint8_t LWM2M_Client::update_routine()
{
	//If client is not registered return from the routine.
	if (client_status == NOT_REGISTERED || client_status == AWAIT_REGISTRATION_RESPONSE) return 1;

	//Do not continue if the update was already sent at this system time to prevent multiple messages
	if (sys_time == last_update_sent) return 1;


	if (client_status == AWAIT_UPDATE_RESPONSE)
	{
		//Check timeout and send the message again if hysteresis is reached.
		if (sys_time >= (lastUpdate + stoi(getObject(1).getResource(1).getValue())) && getObject(1).getResource(1).getValueChanged() != true)
		{
			client_status = NOT_REGISTERED;

			#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_ERROR("Update response not received...");
			#endif
		}

		else if ((sys_time - last_update_sent) % UPDATE_RETRY_TIMEOUT == 0)
		{
			if (sys_time != last_update_sent)
			{
				LOG_INFO("Trigger update 1");
				return send_update();
			}
			return 0;
			
		}
	}
	else
	{
		if (sys_time >= (lastUpdate + stoi(getObject(1).getResource(1).getValue()) - UPDATE_HYSTERESIS))
		{
			LOG_INFO("Trigger update 2");
			return send_update();
		}
	}

	

	return 0;
}


/**
 *	Checks registration timeout
 *
 *	RETURN : Re-registration required
 *
 */
uint8_t LWM2M_Client::registration_routine()
{
	if (sys_time >= last_update_sent + UPDATE_HYSTERESIS) client_status = NOT_REGISTERED;
	return 0;

}


/**
 *	Main function that schedules all previous defined functions and methods. Also checks integrity of an incoming message.
 *
 */
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

	//If registration is currently in progress, check registration timeout
	if (client_status == AWAIT_REGISTRATION_RESPONSE) registration_routine();
	update_routine();
	observe_routine();

	//Check buffer for incoming message
	uint16_t msg_len = getRxData(lw_buffer);
	if (msg_len == NO_DATA_AVAILABLE)
	{
		//If no message was received
		return;
	}

	//if message was received, try parsing it
	if (CoAP_parse_message(&last_message, (char*)lw_buffer, msg_len) != COAP_OK)
	{
		//If there was a error in parsing the message or the message
		//		is not valid, drop the message and return
		return;
	}

	//The message should be parsed correctly here

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
	{
		uint8_t update_message_return = check_update_message(&last_message);
		if (update_message_return == UPDATE_SUCCESS)
		{
#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_INFO("Update successful...");
#endif
			client_status = REGISTERED_IDLE;
			lastUpdate = sys_time;
		}
		else if (update_message_return == UPDATE_FAILED)
		{
			client_status = NOT_REGISTERED;
		}

		else
		{
			process_message(&last_message);
		}
		break;

	}


	case REGISTERED_IDLE: case REGISTERED_TX_DATA_READY: case REGISTERED_AWAIT_RESPONSE: default:
		process_message(&last_message);
		break;
	}

}

/**
 *	Main function used to advance time within the library.
 *
 *	INPUT  : amount_in_seconds - Amount in seconds to increment library (system) time.
 *
 */
void LWM2M_Client::advanceTime(uint16_t amount_in_seconds)
{
	sys_time += amount_in_seconds;
	for(uint8_t i = 0; i < MAX_OBSERVED_ENTITIES; i++)
	{
		if (observed_entities[i].currently_observed == true)
		{
			observed_entities[i].last_notify_sent++;
		}
	}
}


/**
 *	Parses registration message and decides if the registration was successful or not.
 *
 *	RETURN : Registration successful or Registration Failed
 *
 */
uint8_t LWM2M_Client::check_registration_message(CoAP_message_t* c)
{
	if (c->header.type == COAP_ACK)
	{
		if (c->header.returnCode == COAP_SUCCESS_CREATED || c->header.returnCode == COAP_SUCCESS_CHANGED)
		{
			std::string loc_path = CoAP_get_option_string(c, COAP_OPTIONS_LOCATION_PATH);
			if (loc_path.substr(0, 2) == "rd") return REGISTRATION_SUCCESS;
		}
		//TODO: Properly check registration message
	}
	return REGISTRATION_FAILED;
	
}


/**
 *	Parses update message and decides if the update was successful
 *
 *	INPUT  : c - Coap message struct
 *
 *	RETURN : Update successful or Update Failed
 *
 */
uint8_t LWM2M_Client::check_update_message(CoAP_message_t* c)
{
	if (c->header.type == COAP_ACK && c->header.returnCode == COAP_SUCCESS_CHANGED)
	{
		return UPDATE_SUCCESS;
	}
	if (c->header.type == COAP_ACK && c->header.returnCode == COAP_C_ERR_NOT_FOUND)
	{
		return UPDATE_FAILED;
	}
	return NOT_UPDATE_MESSAGE;

}


/**
 *	Parses registration response and saved the registration ID in reg_id variable
 *
 *	INPUT  : c - Coap message struct
 *
 */
void LWM2M_Client::save_registration_id(CoAP_message_t* c)
{
	//TODO: Parse it properly from URI
	for (uint8_t i = 0; i < c->options.options[1].option_length; i++)
	{
		reg_id[i] = c->options.options[1].option_value[i];
	}
	reg_id[c->options.options[1].option_length] = 0;
}

/**
 *	Checks message options and forwards them to the respective interface handle function
 *
 *	INPUT  : c - Coap message struct
 *
 *	RETURN	: 0
 */
uint8_t LWM2M_Client::process_message(CoAP_message_t* c)
{
	print_message_info(c);

	//If the message is reset, it could be a observe reset procedure. Leshan uses RST messages to cancel observe
	if (c->header.type == COAP_RST)
	{
		//For each tag check MID and if found, cancel the observe
		for (uint8_t i = 0; i < MAX_OBSERVED_ENTITIES; i++)
		{

			if (observed_entities[i].observe_mid - 1 == c->header.messageID)
			{
				LOG_INFO("Stoping observe for /" + std::to_string(observed_entities[i].uri.obj_id) + "/" + std::to_string(observed_entities[i].uri.instance_id) + "/" + std::to_string(observed_entities[i].uri.resource_id));
				observed_entities[i].currently_observed = false;
				observed_entities[i].last_notify_sent = 0;
				number_of_observed_entities--;
			}
			if (i == number_of_observed_entities) break;
		}
		return 0;
	}


	char uri_buffer[20];
	CoAP_get_option_chars(c, COAP_OPTIONS_LOCATION_PATH, uri_buffer);

	

	if (uri_buffer[0] == 'r' && uri_buffer[1] == 'd')
	{
		registrationInterfaceHandle(c);
	}
	else if (uri_buffer[0] == 'b' && uri_buffer[1] == 's')
	{
		bootstrapInterfaceHandle(c);
	}
	else
	{
		deviceManagementAndInformationReportingInterfaceHandle(c);
	}

	
	return 0;
}


/**
 *	Debug method that prints basic options of the coap message into the console
 *
 *	INPUT  : c - Coap message struct
 *
 */
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

}

/**
 *	Deregisters client from the LWM2M Server
 *
 *	RETURN : Success or failure.
 */
uint8_t LWM2M_Client::client_deregister() {

	//Check whether client is not already deregistered
	if (client_status == NOT_REGISTERED) {
		LOG_WARNING("client_deregister: Client is not registered to any server");
		return 1;
	}

	//Assemble and send the message
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


/**
 *	Method used to get a specific object. Made public for debug purposes
 *
 *	INPUT  : object_id - ID of Object to get
 *			 instance_id - Instance ID of the object
 *
 *	RETURN : Pointer to the specified object
 *
 */
LWM2M_Object& LWM2M_Client::getObject(uint16_t object_id, uint8_t instance_id)
{
#if defined(USE_VECTORS)
	for(LWM2M_Object& o : objects_vector)
	{
		if (o.getObject_id() == object_id && o.getInstance_id() == instance_id)
			return o;
	}
	return objects_vector.front();
#else
	for (uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id && objects[search_var].getInstance_id() == instance_id)
			return objects[search_var];
	}
	return objects[0];
#endif
	
	
}


/**
 *	Method that handles registration interface messages
 *
 *	INPUT  : c - Coap message struct
 *
 */

void LWM2M_Client::registrationInterfaceHandle(CoAP_message_t* c)
{
	
}


/**
 *	Method that handles bootstrap interface messages
 *
 *	INPUT  : c - Coap message struct
 *
 */
void LWM2M_Client::bootstrapInterfaceHandle(CoAP_message_t* c)
{
	
}


/**
 *	Method that handles device management and information reporting interface messages. Calls respective access methods.
 *
 *	INPUT  : c - Coap message struct
 *
 */
void LWM2M_Client::deviceManagementAndInformationReportingInterfaceHandle(CoAP_message_t* c)
{

	//Parse URI from COAP message
	URI_Path_t uri = CoAP_get_URI(c);

	//Check URI
	bool uri_good = check_URI(&uri);

	//If message is not the same as the last message responded to
	if (uri_good && c->header.messageID != last_mid_responded_to)
	{
		LOG_INFO("Request for " + CoAP_get_option_string(c, COAP_OPTIONS_URI_PATH));

		switch(c->header.returnCode)
		{
		case COAP_METHOD_GET:
		{
				//Check accept option
				std::string s = CoAP_get_option_string(c, COAP_OPTIONS_ACCEPT);

				//if accept option is APP Link format = discover request
				if (s[0] == APPLICATION_LINK_FORMAT)
				{
					lwm_discover(c, &uri);
				}
				else
				{
					lwm_read(c, &uri);
				}
		}
			break;
		case COAP_METHOD_POST:
			lwm_execute(c, &uri);
			break;
		case COAP_METHOD_PUT:
			lwm_write(c, &uri);
			break;
		case COAP_METHOD_DELETE:
			lwm_delete(c, &uri);
			break;
		default: //ACK
			break;
		}
	}

	//Else ignore the message
	else
	{
		//If unknown state occurs return error message
		if (c->header.type != COAP_ACK)
		{
			respond(c, COAP_C_ERR_NOT_FOUND);
			
		}
	}

}

/**
 *	Method that checks whether an object exists within the scope of client.
 *
 *	INPUT : object_id - Object ID
 *			instance_id - Instance ID
 *
 *	OUTPUT: bool - resource exists within this object
 */
bool LWM2M_Client::object_exists(uint16_t object_id)
{
#if defined(USE_VECTORS)
	//For each element in vector
	for(LWM2M_Object& o : objects_vector)
	{
		if (o.getObject_id() == object_id)
			return true;
	}
	return false;
#else
	for (uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id)
			return true;
	}
	return false;
#endif
	
}

bool LWM2M_Client::object_exists(uint16_t object_id, uint16_t instance_id)
{
#if defined(USE_VECTORS)
	//For each element in vector
	for (LWM2M_Object& o : objects_vector)
	{
		if (o.getObject_id() == object_id && o.getInstance_id() == instance_id)
			return true;
	}
	return false;
#else
	for (uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id && objects[search_var].getInstance_id() == instance_id)
			return true;
	}
	return false;
#endif
	
}


/**
 *	Performs an URI check to confirm the target entity exists
 *
 *	INPUT  : uri - URI struct that contains path to the requested entity
 *
 */
bool LWM2M_Client::check_URI(URI_Path_t* uri)
{
	//If Invalid URI depth
	if (uri->path_depth == 0 || (uri->path_depth > 4))
	{
		return false;
	}

	//URI points to object
	if (uri->path_depth >= 1)
	{
		if (!object_exists(uri->obj_id))  return false;
	}

	//URI points to instance
	if (uri->path_depth >= 2)
	{
		if (!object_exists(uri->obj_id, uri->instance_id))  return false;
	}

	//URI points to resource
	if (uri->path_depth >= 3)
	{
		if (!getObject(uri->obj_id, uri->instance_id).resource_exists(uri->resource_id))  return false;
	}

	//URI points to a value within array-type resource
	if (uri->path_depth == 4)
	{
		if (!getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id).value_exists(uri->multi_level_id))  return false;
	}

	//At this point entity exists
	return true;
}


/**
 *	Creates object with specified ID and instance ID and adds it to the vector / array
 *
 *	INPUT  : object_id - ID of Object to get
 *			 instance_id - Instance ID of the object
 *
 */
void LWM2M_Client::createObject(uint16_t object_id, uint8_t instance_id)
{
#if defined(USE_VECTORS)
	objects_vector.push_back(LWM2M_Object(object_id, instance_id));
#else
	object_ids[next_obj_ptr] = object_id;
	LWM2M_Object obj(object_id, instance_id);
	objects[next_obj_ptr] = obj;
	next_obj_ptr++;
#endif
}

/**
 *	Methods used to create a resource within an object. Used to relay information to the Resource class
 *
 *	INPUT : object_id	- Object ID
 *			instance_id	- Instance ID
 *			resource_id - Resource ID
 *			type		- Type of data held in the resource
 *			permissions - Permission of access to the data
 *			multi_level - Whether the resource is a type of field eg. multiple data inside the resource
 *			default_value - Default value.
 *
 **/
void LWM2M_Client::addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value)
{
	if (object_exists(object_id,instance_id))
	{
		getObject(object_id, instance_id).add_resource(resource_id, type, permissions, multi_level, default_value);
	}
}
void LWM2M_Client::addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value)
{
	if (object_exists(object_id, instance_id))
	{
		getObject(object_id, instance_id).add_resource(resource_id, type, permissions, multi_level, default_value);
	}
}
void LWM2M_Client::addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value)
{
	if (object_exists(object_id, instance_id))
	{
		getObject(object_id, instance_id).add_resource(resource_id, type, permissions, multi_level, default_value);
	}
}
void LWM2M_Client::addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value)
{
	if (object_exists(object_id, instance_id))
	{
		getObject(object_id, instance_id).add_resource(resource_id, type, permissions, multi_level, default_value);
	}
}
void LWM2M_Client::addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)())
{
	if (object_exists(object_id, instance_id))
	{
		getObject(object_id, instance_id).add_resource(resource_id, type, permissions, multi_level, execute_func);
	}
}

/**
 *	Performs LWM2M Read Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 */
void LWM2M_Client::lwm_read(CoAP_message_t* c,URI_Path_t* uri)
{
	//At this point URI is good
	
	LOG_INFO("LWM_READ");

	
	bool format_good = check_message_format(c, COAP_OPTIONS_ACCEPT);
	if (!format_good)
	{
		respond(c, COAP_C_ERR_BAD_OPT, std::string("Format not supported"));
		return;
	}


	if (uri->path_depth >= REQUEST_RESOURCE)
	{

		LWM2M_Resource& resource = getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id);
		if (resource.getPermissions() == READ_WRITE || resource.getPermissions() == READ_ONLY)
		{
			send_resource(c, uri, resource);
		}
		else
		{
			//bad permission
			respond(c, COAP_C_ERR_FORBIDDEN, std::string("Invalid access permission."));
		}
	}


	else if (uri->path_depth == REQUEST_INSTANCE)
	{
		LWM2M_Object& instance_object = getObject(uri->obj_id, uri->instance_id);
#if MULTI_VALUE_FORMAT == FORMAT_JSON
		std::string payload = json::createJSON_Instance(instance_object);
#else
		std::string payload = json::createJSON_Instance(instance_object);
#endif
		respond(c, COAP_SUCCESS_CONTENT, payload, MULTI_VALUE_FORMAT);
	}


	else if (uri->path_depth == REQUEST_OBJECT)
	{
		LWM2M_Object& object = getObject(uri->obj_id);
#if MULTI_VALUE_FORMAT == FORMAT_JSON
		std::string payload = json::createJSON_Object(object);
#else
		std::string payload = json::createJSON_Object(object);
#endif
		respond(c, COAP_SUCCESS_CONTENT, payload, MULTI_VALUE_FORMAT);
	}
	//TODO:Add read for instances and objects

}


/**
 *	Performs LWM2M Write Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 */
void LWM2M_Client::lwm_write(CoAP_message_t* c, URI_Path_t* uri)
{
	//At this point URI is good

	LOG_INFO("LWM_WRITE");

	bool format_good = check_message_format(c, COAP_OPTIONS_CONTENT_FORMAT);
	if (!format_good)
	{
		respond(c, COAP_C_ERR_BAD_OPT, std::string("Format not supported"));
		return;
	}

	//json::parseJsonAndUpdate_Resource(uri, c->payload, getObject(1).getResource(1));

	if (uri->path_depth >= REQUEST_RESOURCE)
	{
		LWM2M_Resource& resource = getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id);
		if (resource.getPermissions() == READ_WRITE || resource.getPermissions() == WRITE_ONLY)
		{
#if SINGLE_VALUE_FORMAT == FORMAT_PLAIN_TEXT
			getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id).update_resource(c->payload, uri->multi_level_id);
#else
			json::parseJsonAndUpdate_Resource(uri, c->payload, getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id));
#endif

			//Special cases:
			if (uri->obj_id == 1 && uri->resource_id == 1)
			{
				send_update();
			}
			
			respond(c, COAP_SUCCESS_CHANGED);
		}
		else
		{
			//bad permission
			respond(c, COAP_C_ERR_FORBIDDEN, std::string("Invalid access permission."));
		}
	}
	//TODO:Add write for instances and objects

}

/**
 *	Performs LWM2M Execute Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 */
void LWM2M_Client::lwm_execute(CoAP_message_t* c, URI_Path_t* uri)
{
	if (uri->path_depth >= REQUEST_RESOURCE)
	{
		LWM2M_Resource& resource = getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id);
		if (resource.getPermissions() == EXECUTABLE && resource.getType() == TYPE_EXECUTABLE)
		{
			respond(c, COAP_SUCCESS_CHANGED);

			//Handle special cases
			if (uri->obj_id == 1 && uri->resource_id == 8)
			{
				send_update();
			}
			else
			{
				getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id).execute();
			}
			
		}
		else
		{
			//bad permission
			respond(c, COAP_C_ERR_BAD_REQUEST, std::string("Not an executable resource."));
		}
	}
	else
	{
		respond(c, COAP_C_ERR_NOT_ALLOWED, std::string("Not allowed."));
	}
}

/**
 *	Method used to respond to the incoming message. Used to respond with error codes.
 *
 *	INPUT  : c - Coap message struct of the message to respond to
 *			 return_code - Return code to be sent within the message
 *			 payload - Payload of the response message. Default is none in case of error message
 *			 payload_format - Data format used in payload to set the Format option accordingly.
 *
 */
void LWM2M_Client::respond(CoAP_message_t* c, uint8_t return_code, std::string payload, uint16_t payload_format)
{
	LOG_INFO("Responding to the request. Payload: " + payload);
	CoAP_message_t response;
	int message_type = c->header.type == COAP_CON ? COAP_ACK : COAP_NON;
	CoAP_tx_setup(&response, message_type, c->header.token_length, return_code, c->header.messageID, c->header.token);
	std::string observe = CoAP_get_option_string(c, COAP_OPTIONS_OBSERVE);
	if (observe != "")
	{
		URI_Path_t uri = CoAP_get_URI(c);
		uint8_t entity_id = add_observe_entity(c, &uri);
		CoAP_add_option(&response, COAP_OPTIONS_OBSERVE, observed_entities[entity_id].observed_val);
	}
	if (payload_format != 1) CoAP_add_option(&response, COAP_OPTIONS_CONTENT_FORMAT, payload_format);
	CoAP_set_payload(&response, payload);
	CoAP_assemble_message(&response);
	last_mid_responded_to = c->header.messageID;
	send((char*)(response.raw_data.masg.data()), response.raw_data.message_total);
}


/**
 *	Method used to respond to send a specific resource.
 *
 *	INPUT  : c - Coap message struct of the message to respond to
 *			 uri - URI struct that contains path to the requested entity
 *			 resource - Pointer to a resource
 *
 */
void LWM2M_Client::send_resource(CoAP_message_t* c, URI_Path_t* uri, LWM2M_Resource& resource)
{
	std::string s = CoAP_get_option_string(c, COAP_OPTIONS_ACCEPT);
	uint16_t value_format = 0;
	if (s != "0")
	{
		value_format = s[0] << 8 | s[1];
	}

	if (uri->path_depth == REQUEST_RESOURCE)
	{
		if (resource.getMultiLevel())
		{
#if MULTI_VALUE_FORMAT == FORMAT_JSON
			std::string payload = json::createJSON_Resource(uri, resource);
#else
			std::string payload = json::createJSON_Resource(uri, resource);
#endif

			respond(c, COAP_SUCCESS_CONTENT, payload, MULTI_VALUE_FORMAT);

		}
		else
		{
			std::string payload;
			if (value_format == SINGLE_VALUE_FORMAT)
			{
#if SINGLE_VALUE_FORMAT == FORMAT_PLAIN_TEXT
				payload = resource.getValue(uri->multi_level_id);
#else
				payload = json::createJSON_Resource(uri, resource);
#endif
			}
			else
			{
#if MULTI_VALUE_FORMAT == FORMAT_JSON
				payload = json::createJSON_Resource(uri, resource);
#else
				payload = json::createJSON_Resource(uri, resource);
#endif
			}

			respond(c, COAP_SUCCESS_CONTENT, payload, value_format);

		}
	}
	else //Request multi value resource
	{
		if (resource.getMultiLevel())
		{
			std::string payload;
			if (value_format == SINGLE_VALUE_FORMAT)
			{
#if SINGLE_VALUE_FORMAT == FORMAT_PLAIN_TEXT
				payload = resource.getValue(uri->multi_level_id);
				//resource.clearValueChanged();
#else
				payload = json::createJSON_Resource(uri, resource);
				resource.clearValueChanged();
#endif
		}
			else
			{
#if MULTI_VALUE_FORMAT == FORMAT_JSON
				payload = json::createJSON_Resource(uri, resource);
#else
				payload = json::createJSON_Resource(uri, resource);
#endif
			}

			respond(c, COAP_SUCCESS_CONTENT, payload, value_format);
		}
		else
		{
			//send error
			respond(c, COAP_C_ERR_BAD_REQUEST, std::string("This resource is not a multiresource."));
		}
	}
}




/**
 *	Method used to check whether requested format is supported by the library and the device.
 *
 *	INPUT  : c - Coap message struct of the message to respond to
 *			 option - Option to be checked.
 *
 *	RETURN : True - Format is supported, False - Format is not supported.
 */
bool LWM2M_Client::check_message_format(CoAP_message_t* c, uint16_t option)
{
	std::string s = CoAP_get_option_string(c, option);
	uint16_t value_format = 0;
	if (s != "0")
	{
		value_format = s[0] << 8 | s[1];
	}

	if (value_format != SINGLE_VALUE_FORMAT && value_format != MULTI_VALUE_FORMAT)
	{
		return false;
	}

	return true;
}


/**
 *	Changes value saved inside a resource
 *
 *	INPUT : object_id - Object ID
 *			instance_id - Instance ID
 *			resource_id - Resource ID
 *			value - value in string format
 *			depth - position in array in case of array-type resources
 *
 */
void LWM2M_Client::updateResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, std::string value, uint8_t depth)
{
	if(object_exists(object_id, instance_id))
	{
		if (getObject(object_id, instance_id).resource_exists(resource_id))
		{
			getObject(object_id, instance_id).getResource(resource_id).update_resource(value, depth);

			//Special cases.
			//Lifetime
			if (object_id == 1 && resource_id == 1)
			{
				LOG_INFO("Trigger update 3");
				send_update();
			}
		}
	}
}

/**
 *	Changes value saved inside a resource
 *
 *	INPUT : object_id - Object ID
 *			instance_id - Instance ID
 *			resource_id - Resource ID
 *			value - value in string format
 *			depth - position in array in case of array-type resources
 *
 *	RETURN : Value saved in a resource in a string format
 */
std::string LWM2M_Client::getResourceValue(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t depth)
{
	if (object_exists(object_id, instance_id))
	{
		if (getObject(object_id, instance_id).resource_exists(resource_id))
		{
			return getObject(object_id, instance_id).getResource(resource_id).getValue(depth);
		}
	}
	return "";

}


/**
 *	Method used to create a new observation tag and add it to the field of tags.
 *
 *	INPUT  : c - Coap message struct of the message to respond to
 *			 uri - URI struct that contains path to the requested entity
 *
 *	RETURN : Error or Success.
 */
uint8_t LWM2M_Client::add_observe_entity(CoAP_message_t* c, URI_Path_t* uri)
{
	for(uint8_t n = 0; n < MAX_OBSERVED_ENTITIES; n++)
	{
		if (observed_entities[n].uri.obj_id == uri->obj_id && observed_entities[n].uri.instance_id == uri->instance_id && observed_entities[n].uri.resource_id == uri->resource_id)
			return 0;
	}

	LOG_INFO("Setting observe for: /" + std::to_string(uri->obj_id) + "/" + std::to_string(uri->instance_id) + "/" + std::to_string(uri->resource_id));
	Observed_Entity_t obs;
	obs.observe_depth = uri->path_depth;
	obs.currently_observed = true;
	obs.uri = *uri;
	obs.last_notify_sent = 0;
	obs.observe_mid = rand();//c->header.messageID+1;
	obs.observed_val = rand();

	for(uint8_t i = 0; i < c->header.token_length; i++)
	{
		obs.observe_token[i] = c->header.token[i];
	}

	for(uint8_t i= 0; i < MAX_OBSERVED_ENTITIES; i++)
	{
		if (observed_entities[i].currently_observed == false)
		{
			observed_entities[i] = obs;
			number_of_observed_entities++;
			return i;
		}
	}
	
}

/**
 *	Periodically checks all the observation tags and whether notify should be sent.
 *
 */
void LWM2M_Client::observe_routine()
{
	//Check if client is registered
	if (client_status == NOT_REGISTERED || client_status == AWAIT_REGISTRATION_RESPONSE || client_status == AWAIT_UPDATE_RESPONSE) return;
	uint8_t obs_entities = 0;

	//Go through the tag field
	for(uint8_t i = 0; i < MAX_OBSERVED_ENTITIES; i++)
	{
		//If the entity is currently being observed check attributes
		if (observed_entities[i].currently_observed == true)
		{
			bool is_over_max = observed_entities[i].last_notify_sent >= observed_entities[i].notify_max;
			bool value_changed = false;
			//If the entity is object or instance check all resources for a value change
			if (observed_entities[i].uri.path_depth < REQUEST_RESOURCE)
			{
#if defined(USE_VECTORS)
				for(LWM2M_Resource& r : getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).resources_vector)
				{
					if(r.getValueChanged())
					{
						value_changed = true;
						break;
					}
				}
#else
				for(uint8_t n = 0; n < getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).next_resource_ptr; n++)
				{
					if (getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).resources[n].getValueChanged())
					{
						value_changed = true;
						break;
					}

				}
#endif
			}
			else
			{
				value_changed = getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).getValueChanged();
			}

			//Decide whether notify should be sent
			bool is_over_min = observed_entities[i].last_notify_sent >= observed_entities[i].notify_min;
			bool already_sent = observed_entities[i].last_notify_sent == 0;
			bool notify_should_be_sent = is_over_max || (value_changed && is_over_min) && !already_sent;
			if (notify_should_be_sent)
			{
				//Assemble the message and payload
				CoAP_Message_t notify_message;
				CoAP_tx_setup(&notify_message, COAP_NON, 8, COAP_SUCCESS_CONTENT, observed_entities[i].observe_mid++, observed_entities[i].observe_token);
				CoAP_add_option(&notify_message, COAP_OPTIONS_OBSERVE, observed_entities[i].observed_val++);
				std::string payload;

				//If observed entity is object or instance use MultiValue format
				if (observed_entities[i].observe_depth <= REQUEST_INSTANCE || getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).getMultiLevel() == true)
				{
					if (observed_entities[i].observe_depth == REQUEST_OBJECT) payload = json::createJSON_Object(getObject(observed_entities[i].uri.obj_id));
					else if (observed_entities[i].observe_depth == REQUEST_INSTANCE) payload = json::createJSON_Instance(getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id));
					else if (observed_entities[i].observe_depth == REQUEST_RESOURCE)
					{
						payload = json::createJSON_MVResource(&(observed_entities[i].uri),getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id));
					}
					CoAP_add_option(&notify_message, COAP_OPTIONS_CONTENT_FORMAT, MULTI_VALUE_FORMAT);
				}

				//Use single value format
				else
				{
#if SINGLE_VALUE_FORMAT == FORMAT_PLAIN_TEXT
					payload = getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).getValue();
					if (!(observed_entities[i].uri.obj_id == 1 && observed_entities[i].uri.resource_id == 1)) getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).clearValueChanged();
					CoAP_add_option(&notify_message, COAP_OPTIONS_CONTENT_FORMAT, SINGLE_VALUE_FORMAT);
#else
					payload = json::createJSON_Resource(&(observed_entities[i].uri), getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id));
					if (!(observed_entities[i].uri.obj_id == 1 && observed_entities[i].uri.resource_id == 1)) getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).clearValueChanged();
					CoAP_add_option(&notify_message, COAP_OPTIONS_CONTENT_FORMAT, MULTI_VALUE_FORMAT);
#endif
				}

				CoAP_set_payload(&notify_message, payload);
				CoAP_assemble_message(&notify_message);
				LOG_INFO("Sending Notify for entity: /" + std::to_string(observed_entities[i].uri.obj_id) + "/" + std::to_string(observed_entities[i].uri.instance_id) + "/" + std::to_string(observed_entities[i].uri.resource_id));
				send((char*)(notify_message.raw_data.masg.data()), notify_message.raw_data.message_total);
				observed_entities[i].last_notify_sent = 0;
			}
		}
	}

	
}

/**
 *	Performs LWM2M Discover Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 */
void LWM2M_Client::lwm_discover(CoAP_message_t* c, URI_Path_t* uri)
{
	//if (uri->path_depth == REQUEST)
}

/**
 *
 *	Performs LWM2M Create Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 */
void LWM2M_Client::lwm_create(CoAP_message_t* c, URI_Path_t* uri)
{
	if (uri->path_depth > REQUEST_OBJECT) //Creation os entities other than objects is not allowed (OMA TS. V1.0.2 20180209 page 80)
	{
		respond(c, COAP_C_ERR_NOT_ALLOWED);
	}
	createObject(uri->obj_id);

}

/**
 *	Performs LWM2M Delete Operation as defined by the specification
 *
 *	INPUT  : c - Coap message struct
 *			 uri - URI struct that contains path to the requested entity
 *
 *
 */
void LWM2M_Client::lwm_delete(CoAP_message_t* c, URI_Path_t* uri)
{
	//At this point URI is good
	if (uri->path_depth > REQUEST_INSTANCE || (uri->obj_id == 3 && uri->instance_id == 0)) //Deletes for single resources and Device object 3 is not allowed (OMA TS. V1.0.2 20180209 page 38)
	{
		respond(c, COAP_C_ERR_NOT_ALLOWED);
	}
#if defined(USE_VECTORS)
	uint8_t ctr = 0;
	for(LWM2M_Object& o : objects_vector)
	{
		if (o.getObject_id() == uri->obj_id && o.getInstance_id() == uri->instance_id)
		{
			objects_vector.erase(objects_vector.begin() + ctr);
		}
		ctr++;
	}
#else
	for(uint8_t i = 0; i < next_obj_ptr; i++)
	{
		if (objects[i].getObject_id() == uri->obj_id && objects[i].getInstance_id() == uri->instance_id)
		{
			for(uint8_t m = i; m < next_obj_ptr-1; m++)
			{
				objects[m] = objects[m + 1];
				
			}
			objects[--next_obj_ptr] = LWM2M_Object();
			respond(c, COAP_SUCCESS_DELETED);
			break;
		}
	}
#endif
}




void LWM2M_Client::testMethod()
{
	CoAP_message_t coap_message;
	/*CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_GET, last_mid++);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_HOST, "NwwETjHLuU");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "3");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "0");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "1");
	CoAP_assemble_message(&coap_message);
	send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);*/

	/*URI_Path_t uri = {3,0,0,0,3};
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST, last_mid++);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "dp");
	CoAP_add_option(&coap_message, COAP_OPTIONS_CONTENT_FORMAT, MULTI_VALUE_FORMAT);
	std::string payload = json::createJSON_Resource(&uri,getObject(1).getResource(0));
	CoAP_set_payload(&coap_message, payload);
	CoAP_assemble_message(&coap_message);
	send((char*)(coap_message.raw_data.masg.data()), coap_message.raw_data.message_total);*/

	std::cout << getStatus() << std::endl;
}