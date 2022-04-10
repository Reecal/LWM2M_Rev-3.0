#include "PerformanceTimer.h"

PerformanceTimer::PerformanceTimer(std::string trackedProcess)
{
	m_TrackedProcess = trackedProcess;
	m_StartTimepoint = std::chrono::high_resolution_clock::now();
}

PerformanceTimer::~PerformanceTimer()
{
	Stop();
}

void PerformanceTimer::Stop()
{
	auto endTimepoint = std::chrono::high_resolution_clock::now();
	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

	auto duration = end - start; // in microseconds
	double ms = duration * 0.001;

	std::cout << "Process: " << m_TrackedProcess << " took " << ms << " ms (" << duration << " us) to complete." << std::endl;
}