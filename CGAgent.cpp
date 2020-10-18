#include "CGAgent.hpp"

#include "UltimateTicTacToe.hpp"
#include <iostream>

CGAgent::CGAgent(AgentID id) : Agent(id) {

}

sp<Action> CGAgent::getAction(const up<State>&) {
	int oppRow, oppCol;
	std::cin >> oppRow >> oppCol; std::cin.ignore();
	
	int validActionNum;
	std::cin >> validActionNum; std::cin.ignore();
	for (int i = 0; i < validActionNum; ++i) {
		int row, col;
		std::cin >> row >> col; std::cin.ignore();
	}

	if (oppRow == -1 && oppCol == -1)
		return {};

	int gameRow = oppRow / 3, gameCol = oppCol / 3;
	int row = oppRow % 3, col = oppCol % 3;
	
	return std::mksh<UltimateTicTacToe::UltimateTicTacToeAction>(
		getID(), gameRow, gameCol, TicTacToe::TicTacToeAction(row, col));
}
