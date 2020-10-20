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
	++counter[name];
	isRunning = false;
}

void StatSystem::showStats() const {
	assert(!isRunning);
	assert(numberOfExps != 0);
	printAll();
}

void StatSystem::printAll() const {
	printSeparator();
	printGeneral();
	printDescription();
	printRecords();
	printSeparator();
	std::cout << '\n';
}

void StatSystem::printSeparator() const {
	std::cout << '\n' << std::string(LINE_SEP_LENGTH, '-') << '\n';
}

void StatSystem::printGeneral() const {
	long double msPassed = accumulatedTime * 1e-6;
	std::cout << std::fixed << std::setprecision(2);
	std::cout << '\n';
	std::cout << "Reapeats:\t" << numberOfExps << '\n';
	std::cout << "Total time:\t" << msPassed * 0.001 << " s" << '\n';
	std::cout << std::setprecision(3);
	std::cout << "Avg time:\t" << msPassed / numberOfExps << " ms" << '\n';
}

void StatSystem::printDescription() const {
	for (const auto& [label, vals]: desc)
		if (!vals.empty()) {
			std::cout << '\n' << label << ":\n\n";
			for (const auto& [key, val] : vals)
				if (val == "")
					std::cout << std::string(3, ' ') << key << '\n';
				else
					std::cout << std::string(3, ' ') << key << ": " << val << '\n';
		}
}

void StatSystem::printRecords() const {
	std::cout << std::setprecision(2) << '\n';
	for (const auto& record : counter) {
		if (record.first == "")
			continue;

		double winrate = static_cast<double>(record.second) / numberOfExps * 100;
		std::cout << record.first << ":\t" << winrate << "%" << '\n';
	}
}

void StatSystem::addDesc(const std::string& label,
		const std::vector<KeyValue>& vals) {
	assert(desc.count(label) == 0);
	desc[label] = vals;
}

void StatSystem::reset() {
	accumulatedTime = 0;
	counter.clear();
	desc.clear();	
	numberOfExps = 0;
	isRunning = false;
}
