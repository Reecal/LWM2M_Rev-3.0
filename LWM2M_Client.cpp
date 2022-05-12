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

LWM2M_Client::LWM2M_Client(const char* ep_name, uint8_t(*reb)()) : endpoint_name(ep_name), reboot_cb(reb)
{
	LWM2M_Object obj1(1);
	obj1.add_resource(0, TYPE_INT, READ_ONLY, false, (int) 25);
	obj1.add_resource(1, TYPE_INT, READ_WRITE, false, (int)30);
	obj1.add_resource(6, TYPE_BOOLEAN, READ_WRITE, false, true);
	obj1.add_resource(7, TYPE_STRING, READ_WRITE, false, (char*) "U");
	obj1.add_executable_resource(8);
	object_ids[next_obj_ptr] = 1;
	objects[next_obj_ptr] = obj1;
	next_obj_ptr++;



	LWM2M_Object obj3(3);
	obj3.add_resource(0, TYPE_STRING, READ_ONLY, false, (char*)"Diploma Thesis Radim Dvorak");
	obj3.add_resource(3, TYPE_STRING, READ_ONLY, false, (char*)"Rev 3.0");
	obj3.add_executable_resource(4, reboot_cb);
	obj3.add_resource(11, TYPE_INT, READ_ONLY, true, 0);
	obj3.add_resource(16, TYPE_STRING, READ_ONLY, false, (char*) "U");
	object_ids[next_obj_ptr] = 3;
	objects[next_obj_ptr] = obj3;
	next_obj_ptr++;

	last_mid = rand() % 65536;
}



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

uint16_t LWM2M_Client::getRxData(char*& outputBuffer)
{
	if (!RX_FLAG) return NO_DATA_AVAILABLE;

	char* ptr = rxData[rxBuffer_tail];
	uint16_t msgLen = msg_lengths[rxBuffer_tail];
	//outputBuffer = rxData[rxBuffer_tail];
	outputBuffer = ptr;
	//strcpy_s(outputBuffer, 1000,rxData[rxBuffer_tail]);
	rxBuffer_tail = (rxBuffer_tail + 1) & (RX_BUFFER_MAX_SIZE - 1);

	if (rxBuffer_head == rxBuffer_tail) CLEAR_RX_FLAG();

	return msgLen;
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
	std::string pl = "</>;ct=";
#if MULTI_VALUE_FORMAT == SINGLE_VALUE_FORMAT
	pl += std::to_string(MULTI_VALUE_FORMAT);
#else
	pl += "\"" + std::to_string(SINGLE_VALUE_FORMAT) + " " + std::to_string(MULTI_VALUE_FORMAT) + "\"";
#endif
	pl += ";rt=\"oma.lwm2m\",";

	for(uint8_t i = 0; i < next_obj_ptr; i++)
	{
		pl += "</";
		pl += to_string(objects[i].getObject_id()) + "/" + std::to_string(objects[i].getInstance_id());
		pl += ">";
		if (i != next_obj_ptr - 1) pl += ",";
	}

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
	CoAP_tx_setup(&coap_message, COAP_CON, 8, COAP_METHOD_POST, last_mid++);
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, "rd");
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_PATH, std::string(reg_id));
	CoAP_add_option(&coap_message, COAP_OPTIONS_URI_QUERY, "lt=" + getObject(1).getResource(1).getValue());
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

uint8_t LWM2M_Client::update_routine()
{
	if (client_status == NOT_REGISTERED || client_status == AWAIT_REGISTRATION_RESPONSE) return 1;
	if (client_status == AWAIT_UPDATE_RESPONSE)
	{
		//Try sending update again
		//std::cout << "RETRY: " << (lastUpdate + lifetime) << " : " << (lastUpdate + lifetime) % UPDATE_RETRY_TIMEOUT << std::endl;
		if (sys_time >= (lastUpdate + stoi(getObject(1).getResource(1).getValue())))
		{
			client_status = NOT_REGISTERED;

			#if LOG_OUTPUT == 1 && LOG_VERBOSITY >=1
			LOG_ERROR("Update response not received...");
			#endif
		}

		else if (((lastUpdate + stoi(getObject(1).getResource(1).getValue())) - sys_time) % UPDATE_RETRY_TIMEOUT == 0)
		{
			if (sys_time != last_update_sent)
			{
				return send_update();
			}
			return 0;
			
		}
	}
	else
	{
		if (sys_time >= (lastUpdate + stoi(getObject(1).getResource(1).getValue()) - UPDATE_HYSTERESIS))
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
	if (c->header.type == COAP_ACK && c->header.returnCode == COAP_C_ERR_NOT_FOUND)
	{
		return UPDATE_FAILED;
	}
	return NOT_UPDATE_MESSAGE;

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

uint8_t LWM2M_Client::client_deregister() {
	if (client_status == NOT_REGISTERED) {
		LOG_WARNING("client_deregister: Client is not registered to any server");
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

LWM2M_Object& LWM2M_Client::getObject(uint16_t object_id, uint8_t instance_id)
{
	for(uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id && objects[search_var].getInstance_id() == instance_id)
			return objects[search_var];
	}
	return objects[0];;
}

void LWM2M_Client::registrationInterfaceHandle(CoAP_message_t* c)
{
	
}
void LWM2M_Client::bootstrapInterfaceHandle(CoAP_message_t* c)
{
	
}
void LWM2M_Client::deviceManagementAndInformationReportingInterfaceHandle(CoAP_message_t* c)
{

	if (c->header.type == COAP_RST)
	{
		for (uint8_t i = 0; i < MAX_OBSERVED_ENTITIES; i++)
		{
			if (observed_entities[i].observe_mid-1 == c->header.messageID)
			{
				LOG_INFO("Stoping observe for /" + std::to_string(observed_entities[i].uri.obj_id) + "/" + std::to_string(observed_entities[i].uri.instance_id) + "/" + std::to_string(observed_entities[i].uri.resource_id));
				observed_entities[i].currently_observed = false;
				observed_entities[i].last_notify_sent = 0;
				number_of_observed_entities--;
			}
			if (i == number_of_observed_entities) break;
		}
		return;
	}


	URI_Path_t uri = CoAP_get_URI(c);


	bool uri_good = check_URI(&uri);
	if (uri_good && c->header.messageID != last_mid_responded_to)
	{
		LOG_INFO("Request for " + CoAP_get_option_string(c, COAP_OPTIONS_URI_PATH));

		switch(c->header.returnCode)
		{
		case COAP_METHOD_GET:
			lwm_read(c, &uri);
			break;
		case COAP_METHOD_POST:
			lwm_execute(c, &uri);
			break;
		case COAP_METHOD_PUT:
			lwm_write(c, &uri);
			break;
		default: //DELETE or ACK
			break;
		}
	}

	else
	{
		if (c->header.type != COAP_ACK)
		{
			respond(c, COAP_C_ERR_NOT_FOUND);
			
		}
	}

}

bool LWM2M_Client::object_exists(uint16_t object_id)
{
	for (uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id)
			return true;
	}
	return false;
}

bool LWM2M_Client::object_exists(uint16_t object_id, uint16_t instance_id)
{
	for (uint8_t search_var = 0; search_var < next_obj_ptr; search_var++)
	{
		if (object_ids[search_var] == object_id && objects[search_var].getInstance_id() == instance_id)
			return true;
	}
	return false;
}

bool LWM2M_Client::check_URI(URI_Path_t* uri)
{
	if (uri->path_depth == 0 || (uri->path_depth > 4))
	{
		return false;
	}

	if (uri->path_depth >= 1)
	{
		if (!object_exists(uri->obj_id))  return false;
	}
	if (uri->path_depth >= 2)
	{
		if (!object_exists(uri->obj_id, uri->instance_id))  return false;
	}
	if (uri->path_depth >= 3)
	{
		if (!getObject(uri->obj_id, uri->instance_id).resource_exists(uri->resource_id))  return false;
	}

	if (uri->path_depth == 4)
	{
		if (!getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id).value_exists(uri->multi_level_id))  return false;
	}

	return true;
}

void LWM2M_Client::createObject(uint16_t object_id, uint8_t instance_id)
{
	object_ids[next_obj_ptr] = object_id;
	LWM2M_Object obj(object_id, instance_id);
	objects[next_obj_ptr] = obj;
	next_obj_ptr++;
}

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

	//json::parseJson_Resource(uri, c->payload);

	if (uri->path_depth >= REQUEST_RESOURCE)
	{
		LWM2M_Resource& resource = getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id);
		if (resource.getPermissions() == READ_WRITE || resource.getPermissions() == WRITE_ONLY)
		{
			getObject(uri->obj_id, uri->instance_id).getResource(uri->resource_id).update_resource(c->payload, uri->multi_level_id);

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
			respond(c, COAP_C_ERR_BAD_REQUEST, std::string("Invalid URI."));
		}
	}
}

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
		else
		{
			//send error
			respond(c, COAP_C_ERR_BAD_REQUEST, std::string("This resource is not a multiresource."));
		}
	}
}

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

void LWM2M_Client::updateResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, std::string value, uint8_t depth)
{
	if(object_exists(object_id, instance_id))
	{
		if (getObject(object_id, instance_id).resource_exists(resource_id))
		{
			getObject(object_id, instance_id).getResource(resource_id).update_resource(value, depth);
		}
	}
}

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

void LWM2M_Client::observe_routine()
{
	uint8_t obs_entities = 0;
	for(uint8_t i = 0; i < MAX_OBSERVED_ENTITIES; i++)
	{
		if (observed_entities[i].currently_observed == true)
		{
			bool is_over_max = observed_entities[i].last_notify_sent >= observed_entities[i].notify_max;
			bool value_changed = false;
			if (observed_entities[i].uri.path_depth < REQUEST_RESOURCE)
			{
				for(uint8_t n = 0; n < getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).next_resource_ptr; n++)
				{
					if (getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).resources[n].getValueChanged())
					{
						value_changed = true;
						break;
					}

				}
			}
			else
			{
				value_changed = getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).getValueChanged();
			}
			bool is_over_min = observed_entities[i].last_notify_sent >= observed_entities[i].notify_min;
			bool already_sent = observed_entities[i].last_notify_sent == 0;
			bool notify_should_be_sent = is_over_max || (value_changed && is_over_min) && !already_sent;
			if (notify_should_be_sent)
			{
				CoAP_Message_t notify_message;
				CoAP_tx_setup(&notify_message, COAP_NON, 8, COAP_SUCCESS_CONTENT, observed_entities[i].observe_mid++, observed_entities[i].observe_token);
				CoAP_add_option(&notify_message, COAP_OPTIONS_OBSERVE, observed_entities[i].observed_val++);
				std::string payload;
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
				else
				{
#if SINGLE_VALUE_FORMAT == FORMAT_PLAIN_TEXT
					payload = getObject(observed_entities[i].uri.obj_id, observed_entities[i].uri.instance_id).getResource(observed_entities[i].uri.resource_id).getValue();
					CoAP_add_option(&notify_message, COAP_OPTIONS_CONTENT_FORMAT, SINGLE_VALUE_FORMAT);
#else
					payload = json::createJSON_Resource(&uri, getObject(observed_entities[i].object_id, observed_entities[i].instance_id).getResource(observed_entities[i].resource_id));
#endif
				}

				//CoAP_add_option(&notify_message, COAP_OPTIONS_CONTENT_FORMAT, SINGLE_VALUE_FORMAT);
				CoAP_set_payload(&notify_message, payload);
				CoAP_assemble_message(&notify_message);
				send((char*)(notify_message.raw_data.masg.data()), notify_message.raw_data.message_total);
				observed_entities[i].last_notify_sent = 0;
			}
		}
	}
}