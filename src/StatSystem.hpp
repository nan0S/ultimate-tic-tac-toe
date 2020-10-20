#ifndef STAT_SYSTEM_HPP
#define STAT_SYSTEM_HPP

#include "Common.hpp"

#include <string>
#include <chrono>
#include <vector>
#include <map>

class StatSystem {
public:
	StatSystem();

	void recordStart();
	void recordEnd(const std::string& name);
	void showStats() const;
	void addDesc(const std::string& label,
			const std::vector<KeyValue>& vals);
	void reset();

private:
	void printAll() const;
	void printSeparator() const;
	void printGeneral() const;
	void printDescription() const;
	void printRecords() const;

private:
	static constexpr int LINE_SEP_LENGTH = 30;

	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
	long long accumulatedTime;

	std::map<std::string, int> counter;
	std::map<std::string, std::vector<KeyValue>> desc;

	int numberOfExps;
	bool isRunning;
};

#endif /* STAT_SYSTEM_HPP */
