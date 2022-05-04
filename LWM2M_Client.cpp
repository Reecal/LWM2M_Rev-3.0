#include "LWM2M_Client.h"

#include <cstring>
#include <string.h>

#define SET_TX_FLAG() LWM2M_Client::flags |= 0x01
#define CLEAR_TX_FLAG() LWM2M_Client::flags &= ~0x01

#define SET_RX_FLAG() LWM2M_Client::flags |= 0x02
#define CLEAR_RX_FLAG() LWM2M_Client::flags &= ~0x02

#define RX_FLAG		LWM2M_Client::flags & 0x02


LWM2M_Client::LWM2M_Client(const char* ep_name, uint8_t(*reb)(uint8_t)) : endpoint_name(ep_name), reboot_cb(reb)
{
	
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

	if (txBuffer_head == txBuffer_tail) CLEAR_TX_FLAG();

	if (client_status == REGISTRATION_SCHEDULED) client_status = AWAIT_REGISTRATION_RESPONSE;

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
	return 0;
}

uint8_t LWM2M_Client::getRxData(char* outputBuffer)
{
	if (RX_FLAG) return NO_DATA_AVAILABLE;

	outputBuffer = rxData[rxBuffer_tail];
	rxBuffer_tail = (rxBuffer_tail + 1) & (RX_BUFFER_MAX_SIZE - 1);

	if (rxBuffer_head == rxBuffer_tail) return CLEAR_RX_FLAG();

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
	char dataChar[] = "\x48\x02\x63\xc1\x08\x80\x6e\x17\x2e\xd0\x58\xe5\xb2rd\x11\x28\x33" \
		"b=U\x09lwm2m=1.0\x06lt=100\x0d\x06"\
		"ep=Radim_DP__LWM2M1\xff</>;ct=\"60 110 112 11542 11543\";rt=\"oma.lwm2m\",</1>;ver=1.1,</1/0>,</3>;ver=1.1,</3/0>,</6/0>,</3303>;ver=1.1,</3303/0>,</3441/0>";

#if defined(AUTO_SEND)
	client_status = AWAIT_REGISTRATION_RESPONSE;
	return send(dataChar, strlen(dataChar));
	
#else
	client_status = REGISTRATION_SCHEDULED;
	return schedule_tx(dataChar);
	
#endif


}

void LWM2M_Client::loop()
{
	switch (client_status)
	{
	case NOT_REGISTERED:
#if defined(AUTO_REGISTER)
		send_registration();
		break;
#endif
	case REGISTRATION_SCHEDULED:
		break;
	case AWAIT_REGISTRATION_RESPONSE:
		if (getRxData(lw_buffer) != NO_DATA_AVAILABLE)
		{
			//check if it's registration confirmation, otherwise drop
		}
		break;
	default:
		break;
	}

}