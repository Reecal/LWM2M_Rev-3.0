#pragma once

//#define USE_DTLS

//#define PSK

#define TX_BUFFER_MAX_SIZE 0x08
#define RX_BUFFER_MAX_SIZE 0x08

//#define AUTO_SEND //Whether message should be send automatically via provided send function.

//#define AUTO_REGISTER //Automatically registers to the server. Needs AUTO_SEND to be enabled.

//Supported formats
#define PLAIN_TEXT
//#define JSON
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





