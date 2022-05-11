#pragma once

#include <cstdint>
#include "LWM2M_Defines.h"
#include "CoAP.hpp"
#include "LWM2M_Object.h"
#include "json.h"

class LWM2M_Client
{

	struct Data_In_t
	{
		char* data;
		uint16_t data_length;
	};

	uint8_t number_of_observed_entities = 0;

	struct Observed_Entity_t
	{
		URI_Path_t uri;
		uint8_t observe_depth = 0;
		uint16_t observe_mid = 0;
		uint16_t notify_min = 1;
		uint16_t notify_max = 3600;
		uint16_t observed_val = 0;
		uint16_t last_notify_sent = 0;
		uint8_t observe_token[8] = { 0 };
		uint8_t observe_timer = 0;
		bool currently_observed = false;
	};

	


private:
	LWM2M_Status client_status = NOT_REGISTERED;
	uint8_t flags = 0;

	uint16_t msg_lengths[TX_BUFFER_MAX_SIZE] = { 0 };
	char* txData[TX_BUFFER_MAX_SIZE] = { 0 };
	char* rxData[RX_BUFFER_MAX_SIZE] = { 0 };

	uint8_t rxBuffer_head = 0;
	uint8_t rxBuffer_tail = 0;
	uint8_t txBuffer_head = 0;
	uint8_t txBuffer_tail = 0;
	uint16_t last_mid;
	
	//temp
	uint16_t lifetime = 600;

	long long lastUpdate = 0;
	long long sys_time = 0;

	Observed_Entity_t observed_entities[MAX_OBSERVED_ENTITIES];
	
	const char* endpoint_name;
	char reg_id[12];

	uint16_t last_mid_responded_to = 0;



	uint8_t next_obj_ptr = 0;
	uint16_t object_ids[MAX_OBJECTS * MAX_INSTANCES];
	LWM2M_Object objects[MAX_OBJECTS*MAX_INSTANCES];


	uint8_t(*reboot_cb)();
	uint8_t(*send_cb)(char* data, uint16_t data_len);

	uint8_t schedule_tx(char* data);
	uint16_t getRxData(char*& outputBuffer);
	uint8_t send_registration();
	
	uint8_t update_routine();
	uint8_t check_registration_message(CoAP_message_t* c);
	uint8_t check_update_message(CoAP_message_t* c);
	void save_registration_id(CoAP_message_t* c);
	uint8_t process_message(CoAP_message_t* c);
	void print_message_info(CoAP_message_t* c);
	void registrationInterfaceHandle(CoAP_message_t* c);
	void bootstrapInterfaceHandle(CoAP_message_t* c);
	void deviceManagementAndInformationReportingInterfaceHandle(CoAP_message_t* c);
	bool check_URI(URI_Path_t* uri);
	void lwm_read(CoAP_message_t* c, URI_Path_t* uri);
	void lwm_write(CoAP_message_t* c, URI_Path_t* uri);
	void lwm_execute(CoAP_message_t* c, URI_Path_t* uri);
	void respond(CoAP_message_t* c, uint8_t return_code, std::string payload = "", uint16_t payload_format = 1);
	void send_resource(CoAP_message_t* c, URI_Path_t* uri, LWM2M_Resource& resource);
	bool check_message_format(CoAP_message_t* c, uint16_t option);
	uint8_t add_observe_entity(CoAP_message_t* c, URI_Path_t* uri);
	void observe_routine();

public:
	LWM2M_Client(const char* ep_name, uint8_t(*reb)());
	uint8_t send_update();
	uint8_t receive(char* data, uint16_t data_length);
	LWM2M_Status getStatus();
	uint8_t getTxData(char*& outputBuffer);
	void advanceTime(uint16_t amount_in_seconds);
	LWM2M_Object& getObject(uint16_t object_id, uint8_t instance_id = 0);
	void createObject(uint16_t object_id, uint8_t instance_id);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)());
	bool object_exists(uint16_t object_id);
	bool object_exists(uint16_t object_id, uint16_t instance_id);
	void updateResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, std::string value, uint8_t depth = 0);

	void register_send_callback(uint8_t(*send_func)(char* data, uint16_t data_len));

	//temp
	uint8_t send(char* data, uint16_t data_len);

	void loop();
	uint8_t client_deregister();


};