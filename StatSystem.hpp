#ifndef STAT_SYSTEM_HPP
#define STAT_SYSTEM_HPP

#include <string>
#include <map>
#include <chrono>

class StatSystem {
public:
	StatSystem();

	void recordStart();
	void recordEnd(const std::string& name);
	void showStats();

	void reset();

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
	long long accumulatedTime;
	std::map<std::string, int> stats;
	int numberOfExps;
	bool isRunning;
};

#endif /* STAT_SYSTEM_HPP */
