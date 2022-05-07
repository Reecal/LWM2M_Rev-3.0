#pragma once

#ifndef COAP_H
#define COAP_H

#pragma once
#include <string>

using namespace std;

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <iostream>
//#include <vector>
#include <string>
//#include <winsock2.h>
//#include <sys/types.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>
#include <iostream>


#define COAP_HEADER_VERSION(data)  ( (0xC0 & data[0])>>6    )
#define COAP_HEADER_TYPE(data)     ( (0x30 & data[0])>>4    )
#define COAP_HEADER_TKL(data)      ( (0x0F & data[0])>>0    )
#define COAP_HEADER_CLASS(data)    ( ((data[1]>>5)&0x07)    )
#define COAP_HEADER_CODE(data)     ( ((data[1]>>0)&0x1F)    )
#define COAP_HEADER_MID(data)      ( (data[2]<<8)|(data[3]) )

#define COAP_VERSION					0x1

#define COAP_CON						0x00
#define COAP_NON						0x01
#define COAP_ACK						0x02
#define COAP_RST						0x03

#define COAP_EMPTY_MESSAGE				0x00
#define COAP_METHOD_GET					0x01
#define COAP_METHOD_POST				0x02
#define COAP_METHOD_PUT					0x03
#define COAP_METHOD_DELETE				0x04
#define COAP_METHOD_FETCH				0x05
#define COAP_METHOD_PATCH				0x06
#define COAP_METHOD_IPATCH				0x07

#define COAP_SUCCESS_CREATED			0x41
#define COAP_SUCCESS_DELETED			0x42
#define COAP_SUCCESS_VALID				0x43
#define COAP_SUCCESS_CHANGED			0x44
#define COAP_SUCCESS_CONTENT			0x45

#define COAP_C_ERR_BAD_REQUEST			0x80
#define COAP_C_ERR_UNAUTHORIZED			0x81
#define COAP_C_ERR_BAD_OPT				0x82
#define COAP_C_ERR_FORBIDDEN			0x83
#define COAP_C_ERR_NOT_FOUND			0x84

#define COAP_S_ERR_INTERNAL_ERR			0x90
#define COAP_S_ERR_NOT_IMPLEMENTED		0x91
#define COAP_S_ERR_BAD_GATEWAY			0x92
#define COAP_S_ERR_SERVICE_UNAVAILABLE	0x93
#define COAP_S_ERR_GATEWAY_TIMEOUT		0x94

#define COAP_MAX_OPTION_LENGTH			64
#define COAP_MAX_OPTIONS				10

#define COAP_OPTIONS_IF_MATCH			0x01
#define COAP_OPTIONS_URI_HOST			0x03
#define COAP_OPTIONS_ETAG				0x04
#define COAP_OPTIONS_IF_N_MATCH			0x05
#define COAP_OPTIONS_OBSERVE			0x06
#define COAP_OPTIONS_URI_PORT			0x07
#define COAP_OPTIONS_LOCATION_PATH		0x08
#define COAP_OPTIONS_URI_PATH			0x0b
#define COAP_OPTIONS_CONTENT_FORMAT		0x0c
#define COAP_OPTIONS_MAX_AGE			0x0e
#define COAP_OPTIONS_URI_QUERY			0x0f
#define COAP_OPTIONS_ACCEPT				0x11

#define FORMAT_PLAIN_TEXT				0x00
#define FORMAT_JSON						0x2d17

#define COAP_OK 0
#define COAP_BAD_VERSION 1
#define COAP_BAD_TYPE 2
#define COAP_BAD_TKL 3
#define COAP_BAD_METHOD 4
#define COAP_BAD_OPTION 5

#define COAP_END_OF_OPTIONS 0xff

typedef struct CoAP_Header_t {
	uint8_t type = COAP_CON;
	uint8_t token_length = 0;
	uint8_t returnCode = COAP_S_ERR_INTERNAL_ERR;
	uint16_t messageID = 20158;
	uint8_t token[8] = { 0 };
	
}CoAP_header_t;

typedef struct CoAP_Option_t {
	uint8_t option_number = 0;
	uint8_t option_length = 1;
	uint8_t option_value[COAP_MAX_OPTION_LENGTH] = {0};

}CoAP_option_t;

typedef struct CoAP_Options_t {
	uint8_t number_of_options = 0;
	CoAP_option_t options[COAP_MAX_OPTIONS];

}CoAP_options_t;

typedef struct CoAP_RAW_t {
	uint16_t message_total = 0;
	string masg;

}CoAP_raw_t;

typedef struct CoAP_Message_t {
	CoAP_header_t header;
	CoAP_options_t options;
	string payload;
	CoAP_raw_t raw_data;
}CoAP_message_t;


uint8_t CoAP_init(CoAP_message_t* coap_struct);

uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode);
uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint16_t mid);
uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint8_t token[8]);
uint8_t CoAP_tx_setup(CoAP_message_t* coap_struct, uint8_t type, uint8_t tkl, uint8_t returnCode, uint16_t mid, uint8_t token[8]);

uint8_t CoAP_assemble_token(char buffer[], CoAP_message_t* coap_struct);
uint8_t CoAP_assemble_token(char buffer[], CoAP_message_t* coap_struct, uint8_t token[8]);

uint8_t CoAP_assemble_options(char buffer[], CoAP_message_t* coap_struct);

uint8_t CoAP_add_option(CoAP_message_t* coap_struct, uint8_t optionNumber, string optionArgs);
uint8_t CoAP_add_option(CoAP_message_t* coap_struct, uint8_t optionNumber, int optionArgs);

uint8_t CoAP_set_payload(CoAP_message_t* coap_struct, string payload);

uint8_t CoAP_assemble_message(CoAP_message_t* coap_struct);

uint8_t CoAP_send(SOCKET& sock, CoAP_message_t* coap_struct);

uint8_t CoAP_parse_message(CoAP_message_t* output, char data[], uint16_t msgLen);

string CoAP_get_option_string(CoAP_message_t* coap_struct, uint8_t option_to_get);

void CoAP_send_error_response(SOCKET& sock, CoAP_message_t* message, uint8_t response_code);

uint8_t CoAP_is_valid_coap_message(char data[]);
#endif // !COAP_H

