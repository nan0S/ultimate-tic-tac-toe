#include "Common.hpp"

#include <iomanip>

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
