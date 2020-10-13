#include "StatSystem.hpp"

#include <iostream>
#include <iomanip>
#include <cassert>
#include <ratio>

StatSystem::StatSystem() {
	reset();
}

void StatSystem::recordStart() {
	assert(!isRunning);

	startPoint = std::chrono::high_resolution_clock::now();
	isRunning = true;
}

void StatSystem::recordEnd(const std::string& name) {
	assert(isRunning);

	auto endPoint = std::chrono::high_resolution_clock::now();
	accumulatedTime += std::chrono::duration_cast<
		std::chrono::nanoseconds>(endPoint - startPoint).count();
	++numberOfExps;
	++stats[name];
	isRunning = false;
}

void StatSystem::showStats() {
	assert(!isRunning);
	assert(numberOfExps != 0);

	long double msPassed = accumulatedTime * 1e-6;

	std::cout << std::fixed << std::setprecision(2);
	std::cout << '\n';
	std::cout << "Reapeats:\t" << numberOfExps << '\n';
	std::cout << "Total time:\t" << msPassed << " ms" << '\n';
	std::cout << std::setprecision(3);
	std::cout << "Avg time:\t" << msPassed / numberOfExps << " ms" << '\n';

	std::cout << std::setprecision(2);
	for (const auto& record : stats) {
		if (record.first == "")
			continue;

		double winrate = static_cast<double>(record.second) / numberOfExps * 100;
		std::cout << record.first << ":\t" << winrate << "%" << '\n';
	}

	std::cout << '\n';
}

void StatSystem::reset() {
	accumulatedTime = 0;
	stats.clear();
	numberOfExps = 0;
	isRunning = false;
}
