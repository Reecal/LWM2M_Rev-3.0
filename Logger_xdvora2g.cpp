#include "Logger_xdvora2g.h"
//#pragma once
#/*include <iostream>
#include <vector>
#include <time.h>
#include <thread>*/
#include <string>

bool main_loop_bool = false;

bool LOG123(int logSeverity, std::string entity, std::string info)
{
	LogEvent_t event(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(), logSeverity, entity, info);
	/*event.timestamp = time(NULL);
	event.logEntity = entity;
	event.additionalInfo = info;
	event.logSeverity = logSeverity;*/

	/*m.lock();
	m_logList.emplace_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(), logSeverity, entity, info);
	m.unlock();*/

	Push_Out(event);
	return true;
}

void Push_Out(LogEvent_t& event)
{
	
		//m.lock();
		//bool isEmpty = m_logList.empty();
		//m.unlock();
		//std::this_thread::sleep_for(std::chrono::milliseconds(20));
		
			//m.lock();
			//LogEvent_t event = m_logList[0];
			//m_logList.erase(m_logList.begin());
			//m.unlock();

			double secondsD = event.timestamp * 0.001;
			long long seconds = (long long) secondsD;
			long long miliseconds = (long long)((secondsD - seconds) * 1000);
			long long minutes = secondsD / 60;
			long long hours = minutes / 60;
			seconds = seconds % 60;
			minutes = minutes % 60;
			hours = (hours+1) % 24;

			std::string hourString = std::to_string(hours);
			std::string minutesString = std::to_string(minutes);
			std::string secondsString = std::to_string(seconds);
			std::string milisecondsString = std::to_string(miliseconds);

			if (hours < 10) hourString = '0' + hourString;
			if (minutes < 10) minutesString = '0' + minutesString;
			if (seconds < 10) secondsString = '0' + secondsString;
			if (miliseconds < 10) milisecondsString = "00" + milisecondsString;
			else if (miliseconds < 100) milisecondsString = '0' + milisecondsString;

			
			std::string timestring = "\x1B[33m"+hourString + ":" + minutesString + ":" + secondsString + "." + milisecondsString + "\033[0m";
			


			/*std::cout << "\x1B[32mTexting\033[0m\t\t" << std::endl;
			printf("\x1B[33mTexting\033[0m\t\t");*/
			std::string logType = "";
			if (event.logSeverity == LOG_INFO_MESSAGE_TYPE) logType = "[\x1B[35mINFO\033[0m]: ";
			else if (event.logSeverity == LOG_WARNING_MESSAGE_TYPE) logType = "[\x1B[33mWARN\033[0m]: ";
			else if (event.logSeverity == LOG_ERROR_MESSAGE_TYPE) logType = "[\x1B[31mERR\033[0m ]: ";
			else logType = "[\x1B[32mUNKNOWN\033[0m]: ";



			if (main_loop_bool)
			{
				if (event.logEntity.length()-9 <= 8)
				{
					//m.lock();
					std::cout << "\n[" << timestring << "]" << logType << "\t\t" << event.logEntity << ": " << event.additionalInfo << std::endl << ">";
					//m.unlock();
				}
				/*else if (event.logEntity.length() - 9 < 15)
				{
					m.lock();
					std::cout << "\n[" << timestring << "]" << logType << event.logEntity << ": " << event.additionalInfo << std::endl << ">";
					m.unlock();
				}*/
				else
				{
					//m.lock();
					std::cout << "\n[" << timestring << "]" << logType << "\t" << event.logEntity << ": " << event.additionalInfo << std::endl << ">";
					//m.unlock();
				}
			}
			else
			{
				if (event.logEntity.length()-9 <= 8)
				{
					//m.lock();
					std::cout << "[" << timestring << "]" << logType << "\t\t" << event.logEntity << ": " << event.additionalInfo << std::endl;
					//m.unlock();
				}
				else
				{
					//m.lock();
					std::cout << "[" << timestring << "]" << logType << "\t" << event.logEntity << ": " << event.additionalInfo << std::endl;
					//m.unlock();
				}
				
			}
			





		
		/*int i = 0;
		while (i < 2000) {
			i++;
		}*/
		//std::cout << "Im still here" << std::endl;
//		std::this_thread::yield();
	
}