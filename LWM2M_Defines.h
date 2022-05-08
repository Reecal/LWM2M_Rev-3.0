#pragma once
#include "CoAP.hpp"

#define MAX_OBJECTS 10
#define MAX_INSTANCES 2
#define MAX_RESOURCES 22
#define MAX_RESOURCE_PARTS 5
#define MAX_STRING_LENGTH 40

#define SINGLE_VALUE_FORMAT FORMAT_PLAIN_TEXT
#define MULTI_VALUE_FORMAT	FORMAT_JSON

//#define USE_DTLS

//#define PSK

//These must be multiples of 2 eg. 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 ....
#define TX_BUFFER_MAX_SIZE 0x04
#define RX_BUFFER_MAX_SIZE 0x04

#define AUTO_SEND //Whether message should be send automatically via provided send function.

#define AUTO_REGISTER //Automatically generates registration messages and either sends them, depending on autosend, or schedules send into a buffer

//Supported formats
//#define PLAIN_TEXT
//#define JSON_OMA
//#define TLV

#define UPDATE_HYSTERESIS 10 //Value that determines period before lifetime expires and the update should be sent
#define UPDATE_RETRY_TIMEOUT 4 //Time after which the update request is sent again if no response is received.




enum LWM2M_Status
{
	NOT_REGISTERED,
	REGISTRATION_SCHEDULED,
	AWAIT_REGISTRATION_RESPONSE,
	REGISTERED_IDLE,
	REGISTERED_TX_DATA_READY,
	REGISTERED_AWAIT_RESPONSE,
	AWAIT_UPDATE_RESPONSE,
};


#define DATA_AVAILABLE 1
#define NO_DATA_AVAILABLE 0
#define BUFFER_OVERFLOW 2

#define REGISTRATION_SUCCESS 0
#define REGISTRATION_FAILED 1

#define UPDATE_SUCCESS 0
#define UPDATE_FAILED 1

#define REGISTRATION_INTERFACE 1
#define BOOTSTRAP_INTERFACE 2
#define DEVICE_MANAGEMENT_AND_REPORTING_INTERFACE 3

#define REQUEST_NONE 0
#define REQUEST_OBJECT 1
#define REQUEST_INSTANCE 2
#define REQUEST_RESOURCE 3
#define REQUEST_MV_RESOURCE 4




