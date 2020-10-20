#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <memory>
#include <random>
#include <chrono>

#define mksh make_shared
#define mku make_unique

template<typename T>
using sp = std::shared_ptr<T>;
template<typename T>
using up = std::unique_ptr<T>;
template<typename T>
using wp = std::weak_ptr<T>;

using KeyValue = std::pair<std::string, std::string>;

void errorExit(const std::string& msg);

namespace Random {
	extern std::mt19937 rng;

	template<typename T>
	T rand(T a, T b) {
		using dist_t = std::conditional_t<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>;
		return dist_t{a, b}(rng);
	}

	template<typename T>
	T rand(T n) {
		using dist_t = std::conditional_t<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>;
		if (std::is_integral<T>::value)
			--n;
		return dist_t{0, n}(rng);
	}

	template<typename T, typename Alloc, template<typename, typename> class Container>
	const T& choice(const Container<T, Alloc>& cont) {
		return cont[rand(cont.size())];
	}
}

#define PROFILING 0
#if PROFILING
#define PROFILE_SCOPE(name) SimpleTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

class SimpleTimer {
public:
	SimpleTimer(const std::string& label = "unknown");
	~SimpleTimer();

private:
	static int instanceCounter;

	std::string label;
	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;

	std::string getIndent();
};

class CalcTimer {
public:
	CalcTimer(double limitInMs);

	void startCalculation();
	bool isTimeLeft() const;
	void stopCalculation();

	double getAverageCalcTime() const;
	int getTotalNumberOfCals() const;
	double getLimit() const;
	double getTotalCalcTime() const;

	void changeLimit(double newLimitInMs);

private:
	double getElapsed() const;
	
private:
	double limitInMs;
	bool isRunning = false;
	double totalCalcTime = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	int numberOfCalcs = 0;
};

#endif /* COMMON_HPP */
