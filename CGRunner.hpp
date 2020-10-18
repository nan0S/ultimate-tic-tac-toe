#ifndef CGRUNNER_HPP
#define CGRUNNER_HPP

#include "UltimateTicTacToe.hpp"

class CGRunner {
public:
	CGRunner(double turnLimitInMs);

	void playGame() const;

private:
	void printAction(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const;

private:
	double turnLimitInMs;
	double firstTurnLimitInMs = 1000;
};

#endif /* CGRUNNER_HPP */
