#ifndef CGRUNNER_HPP
#define CGRUNNER_HPP

#include "UltimateTicTacToe.hpp"

class CGRunner {
public:
	CGRunner(double turnLimitInMs);

	void playGame() const;

private:
	void print(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const;

private:
	double turnLimitInMs;
};

#endif /* CGRUNNER_HPP */
