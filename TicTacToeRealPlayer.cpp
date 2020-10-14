#include "TicTacToeRealPlayer.hpp"

#include "UltimateTicTacToe.hpp"

#include <memory>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
using TicTacToeAction = TicTacToe::TicTacToeAction; 

sp<Action> TicTacToeRealPlayer::getAction(const up<State>& state) {
	std::cout << "Type TicTacToe game coords (row, col): "; 
	std::cout.flush();
	int gameRow, gameCol;
	std::cin >> gameRow >> gameCol;

	assert(isInRange(gameRow));
	assert(isInRange(gameCol));

	std::cout << "Type TicTacToe board coords (row, col): "; 
	std::cout.flush();
	int row, col;
	std::cin >> row >> col;

	assert(isInRange(row));
	assert(isInRange(col));

	return std::mksh<UltimateTicTacToeAction>(gameRow - 1, gameCol - 1,
		TicTacToeAction(row - 1, col - 1));
}

bool TicTacToeRealPlayer::isInRange(int idx) const {
	return 1 <= idx && idx <= UltimateTicTacToe::BOARD_SIZE;
}
