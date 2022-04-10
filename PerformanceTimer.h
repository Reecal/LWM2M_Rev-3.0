#pragma once

#include <chrono>
#include <iostream>

class PerformanceTimer
{
public:
	PerformanceTimer(std::string trackedProcess);
	~PerformanceTimer();

	void Stop();

private:

	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
	std::string m_TrackedProcess;
};
