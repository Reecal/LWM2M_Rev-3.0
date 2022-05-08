#include "BG77.h"

#include <string>

#include "Logger_xdvora2g.h"


#if LOG_OUTPUT == 1
#define LOG_ENTITY "\x1B[34mBG77\033[0m"
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




char buffer[1500] = { 0 };

BG77::BG77(bool verbose)
{
	_verbose = verbose;
	echoOff();
}

BG77::~BG77()
{
	
}

int BG77::checkCommunication()
{
	if (_verbose) std::cout << ">AT\r\n";
	sc.sendCommand("AT");
	sc.readSerial(buffer);
	if (_verbose)
	{
		std::cout << buffer;
	}
	
	if (strcmp(buffer, "\r\nOK\r\n"))
	{
		//std::cout << "Wrong - OK\n\r";
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}

int BG77::echoOn()
{
	if (_verbose) std::cout << ">ATE1\r\n";
	sc.sendCommand("ATE1");
	sc.readSerial(buffer);
	if (strcmp(buffer, "\r\nOK\r\n"))
	{
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;

}
int BG77::echoOff()
{
	if (_verbose) std::cout << ">ATE0\r\n";
	sc.sendCommand("ATE0");
	sc.readSerial(buffer);
	if (strcmp(buffer, "\r\nOK\r\n"))
	{
		//std::cout << "Wrong - OK\n\r";
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}

int BG77::checkSIM(int param, char* outBuffer)
{
	switch(param)
	{
	case SIM_ICCID:
		if (_verbose) std::cout << ">AT+QCCID\r\n";
		sc.sendCommand("AT+QCCID");
		sc.readSerial(buffer);

		//std::cout << buffer;
		if (_verbose) std::cout << buffer;

		if (!strstr(buffer, "+QCCID: 8988"))
		{
			return  BG77_INVALID_QCCID;
		}

		memcpy(outBuffer, (void*) &buffer[10], 20);
		outBuffer[20] = '\0';
		return BG77_SUCCESS;

	case SIM_IMSI:
		if (_verbose) std::cout << ">AT+CIMI\r\n";
		sc.sendCommand("AT+CIMI");
		sc.readSerial(buffer);
		if (_verbose) std::cout << buffer;

		if (!strstr(buffer, "\r\nOK\r\n"))
		{
			return BG77_NO_COMMS;
		}

		memcpy(outBuffer, (void*)&buffer[2], 15);
		outBuffer[15] = '\0';
		return BG77_SUCCESS;

	case SIM_READY:
		if (_verbose) std::cout << ">AT+CPIN?\r\n";
		sc.sendCommand("AT+CPIN?");
		sc.readSerial(buffer);
		if (_verbose) std::cout << buffer;

		if (strstr(buffer, "READY"))
		{
			return SIM_IS_READY;
		}
		if (strstr(buffer, "SIM PIN"))
		{
			return SIM_ENTER_PIN;
		}
		if (strstr(buffer, "SIM PUK"))
		{
			return SIM_ENTER_PUK;
		}
		return BG77_NO_COMMS;

	default: 
		return BG77_NO_COMMS;
	}
}
int BG77::lockSIM(int password)
{
	sprintf_s(buffer, "AT+CLCK=\"SC\",1,\"%04d\"", password);
	if (_verbose) std::cout << ">" << buffer << "\r\n";
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;

	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		if (strstr(buffer, "+CME ERROR")) return CME_ERROR;
		return BG77_NO_COMMS;
	}

	return BG77_SUCCESS;
}
int BG77::unlockSIM(int password)
{
	sprintf_s(buffer, "AT+CLCK=\"SC\",0,\"%04d\"", password);
	if (_verbose) std::cout << ">" << buffer << "\r\n";
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;

	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		if (strstr(buffer, "+CME ERROR")) return CME_ERROR;
		return BG77_NO_COMMS;
	}

	return BG77_SUCCESS;
}
int BG77::changePIN(int newPin, int oldPin)
{
	sprintf_s(buffer, "AT+CPWD=\"SC\",\"%04d\",\"%04d\"", oldPin, newPin);
	if (_verbose) std::cout << ">" << buffer << "\r\n";
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;
	if (strstr(buffer, "+CME ERROR"))
	{
		lockSIM(oldPin);
		sprintf_s(buffer, "AT+CPWD=\"SC\",\"%04d\",\"%04d\"", oldPin, newPin);
		if (_verbose) std::cout << ">" << buffer << "\r\n";
		sc.sendCommand(buffer);
		sc.readSerial(buffer);
		if (_verbose) std::cout << buffer;
		if (!strstr(buffer, "\r\nOK\r\n"))
		{
			return BG77_NO_COMMS;
		}
		//return BG77_SUCCESS;
	}
	return BG77_SUCCESS;
}

int BG77::sendRAW(const char input[], char* outputBuffer)
{
	if (_verbose) std::cout << ">" << input <<"\r\n";
	sc.sendCommand(input);
	sc.readSerial(outputBuffer);
	return 0;
}

int BG77::readData(char* outputBuffer)
{
	if (_verbose)std::cout << "Read data\r\n";
	sc.readSerial(outputBuffer);
	if (_verbose)std::cout << outputBuffer;
	if (!strstr(buffer, "+QIURC")) return BG77_INCOMING_DATA;
	return  BG77_NO_DATA;
}

int BG77::pullData(char* outputBuffer)
{
	if (_verbose)std::cout << "Pull data\r\n";
	char messageBuf[1500];
	sendRAW("AT+QIRD=1", messageBuf);
	if (_verbose)std::cout << messageBuf;
	if (messageBuf[0] == 0) return 0;
	int ptr = strstr(messageBuf, "\r\n+QIRD") - messageBuf + 9;
	if (ptr < 0) return 0;
	if (messageBuf[ptr] == '0' || messageBuf[ptr] == 0) return 0;
	int numbfr[4];
	int index = 0;
	
	while(messageBuf[ptr] != ',' && index <= 4)
	{
		numbfr[index++] = messageBuf[ptr++] - 0x30;
	}

	int message_len = 0;
	for(int n = 0; n < index; n++)
	{
		int timer = 1;
		for(int ctr = n ; ctr < index-1; ctr++)
		{
			timer *= 10;
		}
		message_len += numbfr[n] * timer;
	}

	char* beginptr = strstr(messageBuf + ptr, "\r\n");
	beginptr += 2;

	for(int c = 0; c < message_len; c++)
	{
		outputBuffer[c] = beginptr[c];
	}
	outputBuffer[message_len] = 0;
	if (_verbose)std::cout << message_len;
	
	LOG_INFO("Message received: Message length: " + std::to_string(message_len));



	return message_len;
}

int BG77::activatePDP(uint8_t PDP_index)
{
	LOG_INFO("Opening PDP session.");
	sprintf_s(buffer, "AT+QIACT=%d", PDP_index);
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;
	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		LOG_ERROR("Opening PDP FAILED.");
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}

int BG77::openSocket(uint8_t PDP_index, uint8_t port)
{
	LOG_INFO("Opening Socket.");
	sprintf_s(buffer, "AT+QIOPEN=%d,1,\"UDP SERVICE\",\"127.0.0.1\",0,%d,0", PDP_index,port);
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;
	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		LOG_ERROR("Opening Socket FAILED.");
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}

int BG77::deactivatePDP(uint8_t PDP_index)
{
	LOG_INFO("Closing PDP session.");
	sprintf_s(buffer, "AT+QIDEACT=%d", PDP_index);
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;
	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		LOG_ERROR("Closing PDP FAILED.");
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}

int BG77::closeSocket(uint8_t PDP_index)
{
	LOG_INFO("Closing Socket.");
	sprintf_s(buffer, "AT+QICLOSE=%d", PDP_index);
	sc.sendCommand(buffer);
	sc.readSerial(buffer);
	if (_verbose) std::cout << buffer;
	if (!strstr(buffer, "\r\nOK\r\n"))
	{
		LOG_ERROR("Closing Socket FAILED.");
		return BG77_NO_COMMS;
	}
	return BG77_SUCCESS;
}
