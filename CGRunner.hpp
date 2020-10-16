#ifndef CGRUNNER_HPP
#define CGRUNNER_HPP

#include "UltimateTicTacToe.hpp"

class CGRunner {
public:
	void playGame() const;

private:
	void print(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const;
};

#endif /* CGRUNNER_HPP */
