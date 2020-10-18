#include "Common.hpp"

#include <iomanip>
#include <cassert>

void errorExit(const std::string& msg) {
	std::cerr << "ERROR: " << msg << '\n';
	exit(EXIT_FAILURE);
}

namespace Random {
	std::mt19937 rng(std::random_device{}());
}

int SimpleTimer::instanceCounter = 0;

SimpleTimer::SimpleTimer(const std::string& label) : label(label) {
	startPoint = std::chrono::high_resolution_clock::now();
	std::cout << getIndent() << "PROFILING(" << label << ")" << '\n';
	++instanceCounter;
}

SimpleTimer::~SimpleTimer() {
	--instanceCounter;
	auto endPoint = std::chrono::high_resolution_clock::now();
	long double elapsed = std::chrono::duration_cast<
		std::chrono::nanoseconds>(endPoint - startPoint).count() * 1e-6;
	std::cout << std::fixed << std::setprecision(5);
	std::cout << getIndent() << label << ":\t" << elapsed << " ms" << '\n';
}

std::string SimpleTimer::getIndent() {
	return std::string(2 * instanceCounter, ' ');
}

CalcTimer::CalcTimer(double limitInMs) : limitInMs(limitInMs) {
}

void CalcTimer::startCalculation() {
	assert(!isRunning);
	startTime = std::chrono::high_resolution_clock::now();
	isRunning = true;
}

bool CalcTimer::isTimeLeft() const {
	return getElapsed() <= limitInMs;
}

double CalcTimer::getElapsed() const {
	auto endTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<
		std::chrono::nanoseconds>(endTime - startTime).count() * 1e-6;
}


void CalcTimer::endCalculation() {
	assert(isRunning);
	totalCalcTime += getElapsed();
	isRunning = false;
	++numberOfCalcs;
}

double CalcTimer::getAverageCalcTime() const {
	assert(numberOfCalcs != 0);
	return totalCalcTime / numberOfCalcs;
}

int CalcTimer::getTotalNumberOfCals() const {
	return numberOfCalcs;
}

double CalcTimer::getLimit() const {
	return limitInMs;
}
