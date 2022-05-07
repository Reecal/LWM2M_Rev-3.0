#include "BG77.h"

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