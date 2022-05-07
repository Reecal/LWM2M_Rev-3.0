#include "SerialComm.h"
#include <iostream>

SerialComm::SerialComm()
{
    LPCWSTR lpcwstr = L"\\\\.\\COM6";
    serialHandle = CreateFile(lpcwstr, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // Do some basic settings
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(serialHandle, &serialParams);
    serialParams.BaudRate = 115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = 1;
    serialParams.Parity = 0;
    SetCommState(serialHandle, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(serialHandle, &timeout);
}

SerialComm::~SerialComm()
{
    CloseHandle(serialHandle);
}

void SerialComm::sendCommand(const char input[])
{
    char buffer[1500];
    sprintf_s(buffer, "%s\r\n", input);
    //std::cout << ">" << buffer;
    WriteFile(serialHandle, buffer, strlen(buffer), p, d);
}

void SerialComm::readSerial(char* outputBuffer, int bufferSize)
{
    for(int i = 0; i < bufferSize; i++)
    {
        outputBuffer[i] = 0;
    }
	bool read_success = ReadFile(serialHandle, outputBuffer, bufferSize, p, d);
    //std::cout << outputBuffer << std::endl;
}