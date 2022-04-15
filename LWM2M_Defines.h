#pragma once

//#define USE_DTLS

//#define PSK

//These must be multiples of 2 eg. 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 ....
#define TX_BUFFER_MAX_SIZE 0x04
#define RX_BUFFER_MAX_SIZE 0x04

//#define AUTO_SEND //Whether message should be send automatically via provided send function.

//#define AUTO_REGISTER //Automatically registers to the server. Needs AUTO_SEND to be enabled.

//Supported formats
#define PLAIN_TEXT
//#define JSON_OMA
//#define TLV



enum LWM2M_Status
{
	NOT_REGISTERED,
	AWAIT_REGISTRATION_RESPONSE,
	REGISTERED_IDLE,
	REGISTERED_TX_DATA_READY,
	REGISTERED_AWAIT_RESPONSE,
};


#define NO_DATA_AVAILABLE 1
#define BUFFER_OVERFLOW 2





