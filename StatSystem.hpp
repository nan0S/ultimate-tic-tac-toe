#ifndef STAT_SYSTEM_HPP
#define STAT_SYSTEM_HPP

#include <string>
#include <map>
#include <chrono>
#include <vector>

class StatSystem {
public:
	StatSystem();

	void recordStart();
	void recordEnd(const std::string& name);
	void showStats() const;
	void addDesc(const std::string& label,
			const std::map<std::string, std::string>& vals);
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
	std::map<std::string, std::map<std::string, std::string>> desc;

	int numberOfExps;
	bool isRunning;
};

#endif /* STAT_SYSTEM_HPP */
