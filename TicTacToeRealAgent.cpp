#include "TicTacToeRealAgent.hpp"

#include "UltimateTicTacToe.hpp"

#include <memory>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
using TicTacToeAction = TicTacToe::TicTacToeAction; 

TicTacToeRealAgent::TicTacToeRealAgent(AgentID id, double calcLimitInMs,
	const up<State>&, const AgentArgs&) : Agent(id, calcLimitInMs) {

}

sp<Action> TicTacToeRealAgent::getAction(const up<State>& state) {
	while (true) {
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

		auto action = std::mksh<UltimateTicTacToeAction>(
			getID(), gameRow - 1, gameCol - 1,
			TicTacToeAction(row - 1, col - 1));

		if (state->isValid(action))
			std::cout << "You typed invalid action. Try again.\n";
		else
			return action;
	}
	assert(false);
}

bool TicTacToeRealAgent::isInRange(int idx) const {
	return 1 <= idx && idx <= UltimateTicTacToe::BOARD_SIZE;
}

std::vector<KeyValue> TicTacToeRealAgent::getDesc(double) const {
	return { { "Real world, interactive player.", "" } };
}
