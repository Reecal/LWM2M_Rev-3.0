#pragma once

#include <iostream>
#include <chrono>

#define LOG_INFO_MESSAGE_TYPE		0
#define LOG_WARNING_MESSAGE_TYPE	1
#define LOG_ERROR_MESSAGE_TYPE		2




#define CET 1
#define CEST 2

#define UTC_OFFSET CEST



#define DIFF_SUMMER_AND_WINTER_TIME


struct LogEvent_t {
	unsigned long long timestamp;
	int logSeverity;
	std::string logEntity;
	std::string additionalInfo;

	LogEvent_t(unsigned long long t, int lS, std::string lE, std::string aI)
	{
		timestamp = t;
		logSeverity = lS;
		logEntity = lE;
		additionalInfo = aI;
	}
};

bool LOG123(int logSeverity, std::string entity, std::string info);
void Push_Out(LogEvent_t& event);


