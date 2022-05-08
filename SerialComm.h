#pragma once

#include <cstdint>

#include "windows.h"

class SerialComm
{
private:
	HANDLE serialHandle;
	LPDWORD p = 0;
	LPOVERLAPPED d = 0;

public:
	SerialComm();
	~SerialComm();
	void sendCommand(const char input[]);
	void readSerial(char* outputBuffer, int bufferSize=1500);
	void sendData(const char input[], uint16_t data_len);


};
