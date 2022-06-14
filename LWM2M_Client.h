#pragma once

#include <cstdint>
#include <vector>
#include <stdlib.h>
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

	
	//TODO Shrink the tag to use less bytes in memory.
	struct Observed_Entity_t
	{
		URI_Path_t uri;							//Uri of observed entity
		uint8_t observe_depth = 0;				//Value used to determine whether observed entity is an object, instance, resource or a single value within a multi value resource
		uint16_t observe_mid = 0;				//MID for observe - required by Leshan
		uint16_t notify_min = 1;				//Default Minimum Notify interval
		uint16_t notify_max = 15;				//Default Maximum Notify interval
		uint16_t observed_val = 0;
		uint16_t last_notify_sent = 0;			//Timestamp of the last notify sent - used to shedule sending of a notify message
		uint8_t observe_token[8] = { 0 };		//Observe token assigned to the observation - required by Leshan
		uint8_t observe_timer = 0;				//Counts time from the last norification that was sent
		bool currently_observed = false;		//Flag to determine whether this entity is still being observed
	};

	


private:
	LWM2M_Status client_status = NOT_REGISTERED;		//Status of a client
	uint8_t flags = 0;									//Flags used to announce reception of data or that the data is ready to be sent

	uint16_t msg_lengths[TX_BUFFER_MAX_SIZE] = { 0 };	//Field that hold length of received data
	char* txData[TX_BUFFER_MAX_SIZE] = { 0 };			//Buffer for TX
	char* rxData[RX_BUFFER_MAX_SIZE] = { 0 };			//Buffer for RX
													
	//Circular buffer variables
	uint8_t rxBuffer_head = 0;							
	uint8_t rxBuffer_tail = 0;
	uint8_t txBuffer_head = 0;
	uint8_t txBuffer_tail = 0;
	uint16_t last_mid;

	uint8_t number_of_observed_entities = 0;			//Indicates current number of observed entities 
	
	//temp
	uint16_t lifetime = 600;							//Default lifetime - currently not used.

	long long lastUpdate = 0;							//Timestamp of the last update
	long long sys_time = 0;								//Current library time

	Observed_Entity_t observed_entities[MAX_OBSERVED_ENTITIES];	//Field that contains observe tokens
	
	const char* endpoint_name;							//Endpoint name 
	char reg_id[12];									//Registration ID assigned to the device by the server

	uint16_t last_mid_responded_to = 0;					//Variable to hold MID of the last message that was responded to to prevent multiple TX of data
#if defined(USE_VECTORS)
	vector<LWM2M_Object> objects_vector;				//Vector that holds all objects
#else
	uint8_t next_obj_ptr = 0;
	uint16_t object_ids[MAX_OBJECTS * MAX_INSTANCES]; 
	LWM2M_Object objects[MAX_OBJECTS*MAX_INSTANCES];
#endif


	uint8_t(*reboot_cb)();								//Pointer to a callback function
	uint8_t(*send_cb)(char* data, uint16_t data_len);	//Pointer to a send function


	/**
	 *	Schedules TX of data to the TX buffer and sets corresponding flags
	 *
	 *	INPUT : data - data to be sent
	 *
	 *	RETURN : Success or Error code
	 *
	 */
	uint8_t schedule_tx(char* data);

	/**
	 *	Reads incoming data
	 *
	 *	INPUT : outputBuffer - buffer in which read data will be copied to
	 *
	 *	RETURN : Data available or No data available and their respective codes.
	 *
	 */
	uint16_t getRxData(char*& outputBuffer);

	/**
	 *	Directly sends or schedules a TX with a registration message to the server
	 *	
	 *	RETURN : Success or Error code
	 *
	 */
	uint8_t send_registration();

	/**
	 *	Checks last update or registration timings and decides whether an update should be sent
	 *
	 *	RETURN : Update sent or not
	 *
	 */
	uint8_t update_routine();

	/**
	 *	Checks registration timeout
	 *
	 *	RETURN : Re-registration required
	 *
	 */
	uint8_t registration_routine();

	/**
	 *	Parses registration message and decides if the registration was successful or not.
	 *
	 *	RETURN : Registration successful or Registration Failed
	 *
	 */
	uint8_t check_registration_message(CoAP_message_t* c);

	/**
	 *	Parses update message and decides if the update was successful
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 *	RETURN : Update successful or Update Failed
	 *
	 */
	uint8_t check_update_message(CoAP_message_t* c);

	/**
	 *	Parses registration response and saved the registration ID in reg_id variable
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 */
	void save_registration_id(CoAP_message_t* c);

	/**
	 *	Checks message options and forwards them to the respective interface handle function
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 *	RETURN	: 0
	 */
	uint8_t process_message(CoAP_message_t* c);

	/**
	 *	Debug method that prints basic options of the coap message into the console
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 */
	void print_message_info(CoAP_message_t* c);

	/**
	 *	Method that handles registration interface messages
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 */
	void registrationInterfaceHandle(CoAP_message_t* c);

	/**
	 *	Method that handles bootstrap interface messages
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 */
	void bootstrapInterfaceHandle(CoAP_message_t* c);

	/**
	 *	Method that handles device management and information reporting interface messages. Calls respective access methods.
	 *
	 *	INPUT  : c - Coap message struct
	 *
	 */
	void deviceManagementAndInformationReportingInterfaceHandle(CoAP_message_t* c);

	/**
	 *	Performs an URI check to confirm the target entity exists
	 *
	 *	INPUT  : uri - URI struct that contains path to the requested entity
	 *
	 */
	bool check_URI(URI_Path_t* uri);

	/**
	 *	Performs LWM2M Read Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 */
	void lwm_read(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *	Performs LWM2M Write Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 */
	void lwm_write(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *	Performs LWM2M Execute Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 */
	void lwm_execute(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *	Method used to respond to the incoming message. Used to respond with error codes.
	 *
	 *	INPUT  : c - Coap message struct of the message to respond to
	 *			 return_code - Return code to be sent within the message
	 *			 payload - Payload of the response message. Default is none in case of error message
	 *			 payload_format - Data format used in payload to set the Format option accordingly.
	 *
	 */
	void respond(CoAP_message_t* c, uint8_t return_code, std::string payload = "", uint16_t payload_format = 1);

	/**
	 *	Method used to respond to send a specific resource.
	 *
	 *	INPUT  : c - Coap message struct of the message to respond to
	 *			 uri - URI struct that contains path to the requested entity
	 *			 resource - Pointer to a resource
	 *
	 */
	void send_resource(CoAP_message_t* c, URI_Path_t* uri, LWM2M_Resource& resource);

	/**
	 *	Method used to check whether requested format is supported by the library and the device.
	 *
	 *	INPUT  : c - Coap message struct of the message to respond to
	 *			 option - Option to be checked.
	 *
	 *	RETURN : True - Format is supported, False - Format is not supported.
	 */
	bool check_message_format(CoAP_message_t* c, uint16_t option);

	/**
	 *	Method used to create a new observation tag and add it to the field of tags.
	 *
	 *	INPUT  : c - Coap message struct of the message to respond to
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 *	RETURN : Error or Success.
	 */
	uint8_t add_observe_entity(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *	Periodically checks all the observation tags and whether notify should be sent.
	 *
	 */
	void observe_routine();

	/**
	 *	Performs LWM2M Discover Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 */
	void lwm_discover(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *	Performs LWM2M Delete Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *
	 *	
	 */
	 //TODO Implement this method according to the standard
	void lwm_delete(CoAP_message_t* c, URI_Path_t* uri);

	/**
	 *
	 *	Performs LWM2M Create Operation as defined by the specification
	 *
	 *	INPUT  : c - Coap message struct
	 *			 uri - URI struct that contains path to the requested entity
	 *	
	 */
	 //TODO Implement this method according to the standard
	void lwm_create(CoAP_message_t* c, URI_Path_t* uri);

		
public:
	/**
	 *	Default constructor for the Client class
	 *
	 *	INPUT  : ep_name - Endpoint name - Name of the device
	 *			 reb - Pointer to a reboot function
	 *
	 */
	LWM2M_Client(const char* ep_name, uint8_t(*reb)());

	/**
	 *	Directly sends or schedules update
	 *
	 *	RETURN : Success or failure
	 *
	 */
	uint8_t send_update();

	/**
	 *	Main function to process incoming message.
	 *	
	 *	INPUT  : data - Data to be copied into receive buffer
	 *			 data_length - length of data
	 *
	 *	RETURN : Success or failure (Buffer Overflow)
	 *
	 */
	uint8_t receive(char* data, uint16_t data_length);

	/**
	 *	Query for the current status of the client.
	 *
	 *	RETURN : Client status
	 *
	 */
	LWM2M_Status getStatus();

	/**
	 *	Copies data from the TX buffer to the outputBuffer. If it is the last message, clears TX flag.
	 *
	 *	INPUT  : outputBuffer - output buffer to which data will be copied
	 *	RETURN : Success or failure (Buffer overflow)
	 *
	 */
	uint8_t getTxData(char*& outputBuffer);

	/**
	 *	Main function used to advance time within the library.
	 *
	 *	INPUT  : amount_in_seconds - Amount in seconds to increment library (system) time.
	 *
	 */
	void advanceTime(uint16_t amount_in_seconds);

	/**
	 *	Method used to get a specific object. Made public for debug purposes
	 *
	 *	INPUT  : object_id - ID of Object to get
	 *			 instance_id - Instance ID of the object
	 *
	 *	RETURN : Pointer to the specified object
	 *
	 */
	LWM2M_Object& getObject(uint16_t object_id, uint8_t instance_id = 0);

	/**
	 *	Creates object with specified ID and instance ID and adds it to the vector / array
	 *
	 *	INPUT  : object_id - ID of Object to get
	 *			 instance_id - Instance ID of the object
	 *
	 */
	void createObject(uint16_t object_id, uint8_t instance_id = 0);

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
	//TODO Consolidate these methods to a single method using single argument as data
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, float default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, bool default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, int default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, char* default_value);
	void addResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t type, uint8_t permissions, bool multi_level, uint8_t(*execute_func)());

	/**
	 *	Method that checks whether an object exists within the scope of client.
	 *
	 *	INPUT : object_id - Object ID
	 *			instance_id - Instance ID
	 *
	 *	OUTPUT: bool - resource exists within this object
	 */
	bool object_exists(uint16_t object_id);
	bool object_exists(uint16_t object_id, uint16_t instance_id);

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
	void updateResource(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, std::string value, uint8_t depth = 0);

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
	std::string getResourceValue(uint16_t object_id, uint8_t instance_id, uint16_t resource_id, uint8_t depth = 0);

	void testMethod();

	/**
	 *	Registers and saved pointer to a function used to send data
	 *
	 *	INPUT : send_func - Pointer to a send function. Has to have two inputs. Data and data_length
	 *		
	 *
	 *	RETURN : Value saved in a resource in a string format
	 */
	void register_send_callback(uint8_t(*send_func)(char* data, uint16_t data_len));

	//temp
	uint8_t send(char* data, uint16_t data_len);

	/**
	 *	Main function that schedules all previous defined functions and methods. Also checks integrity of an incoming message.
	 *
	 */
	void loop();

	/**
	 *	Deregisters client from the LWM2M Server
	 *
	 *	RETURN : Success or failure.
	 */
	uint8_t client_deregister();


};