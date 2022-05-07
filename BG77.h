#pragma once

#include "SerialComm.h"
#include <iostream>
#include <stdio.h>

#define SIM_ICCID	1
#define SIM_READY	2
#define SIM_LOCK	3
#define SIM_IMSI	4

#define SIM_IS_READY	0
#define SIM_ENTER_PIN	11
#define SIM_ENTER_PUK	12

#define BG77_NO_COMMS	-1
#define BG77_INVALID_QCCID -2
#define BG77_SUCCESS	0

#define CME_ERROR -3




class BG77
{
private:
	SerialComm sc;
	bool _verbose;

public:
	BG77(bool verbose = false);
	~BG77();

	int checkCommunication();

	int echoOn();
	int echoOff();

	int checkSIM(int param, char* outBuffer = nullptr);
	int lockSIM(int password);
	int unlockSIM(int password);
	int changePIN(int newPin, int oldPin);

	int sendRAW(const char input[], char* outputBuffer);



};
