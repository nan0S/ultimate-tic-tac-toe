#include "UltimateTicTacToe.hpp"

#include <algorithm>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;

UltimateTicTacToeAction::UltimateTicTacToeAction(const AgentID& agentID, int row, int col,
		const TicTacToe::TicTacToeAction& action) :
	agentID(agentID), row(row), col(col), action(action) {
	PROFILE_FUNCTION();
}
						
bool UltimateTicTacToe::isTerminal() const {
	PROFILE_FUNCTION();

	if (isAllTerminal())
		return true;
	for (int i = 0; i < BOARD_SIZE; ++i)
		if (isRowDone(i) || isColDone(i))
			return true;
	return isDiag1Done() || isDiag2Done();
}

bool UltimateTicTacToe::isAllTerminal() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i)
		if (!std::all_of(board[i], board[i] + BOARD_SIZE,
					[](const auto& x){ return x.isTerminal(); }))
			return false;
	return true;
}

bool UltimateTicTacToe::isRowDone(int row) const {
	PROFILE_FUNCTION();

	const auto winner = board[row][0].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[row][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isColDone(int col) const {
	PROFILE_FUNCTION();

	const auto winner = board[0][col].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][col].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag1Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][0].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag2Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][BOARD_SIZE - 1].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][BOARD_SIZE - 1 - i].getWinner() != winner)
			return false;
	return true;
}

void UltimateTicTacToe::apply(const sp<Action>& act) {
	PROFILE_FUNCTION();

	const auto& action = std::dynamic_pointer_cast<UltimateTicTacToeAction>(act);
	assert(action);
	assert(isLegal(action));

	board[action->row][action->col].apply(turn, action->action);
	turn = turn == AGENT1 ? AGENT2 : AGENT1;
	
	if (board[action->action.row][action->action.col].isTerminal())
		lastRow = lastCol = -1;
	else
		lastRow = action->action.row,
		lastCol = action->action.col;
}

bool UltimateTicTacToe::isLegal(const sp<UltimateTicTacToeAction>& action) const {
	PROFILE_FUNCTION();

	return canMove(action->agentID) &&
		  isInRange(action->row) &&
		  isInRange(action->col) &&
		  properBoard(action->row, action->col) && 
		  board[action->row][action->col].isLegal(action->action);
}

bool UltimateTicTacToe::canMove(AgentID agentID) const {
	return agentID == NONE || agentID == turn;
}

bool UltimateTicTacToe::isInRange(int idx) const {
	PROFILE_FUNCTION();

	return 0 <= idx && idx < BOARD_SIZE;
}

bool UltimateTicTacToe::properBoard(int boardRow, int boardCol) const {
	return (lastRow == -1 && lastCol == -1) || 
		(boardRow == lastRow && boardCol == lastCol);
}

std::vector<sp<Action>> UltimateTicTacToe::getValidActions() {
	PROFILE_FUNCTION();

	std::vector<sp<Action>> validActions;

	if (lastRow == -1 && lastCol == -1) {
		for (int i = 0; i < BOARD_SIZE; ++i)
			for (int j = 0; j < BOARD_SIZE; ++j) {
				const auto& cell = board[i][j];
				if (cell.isTerminal())
					continue;
				for (int k = 0; k < BOARD_SIZE; ++k)
					for (int l = 0; l < BOARD_SIZE; ++l)
						if (cell.isEmpty(k, l))
							validActions.push_back(std::mksh<UltimateTicTacToeAction>(
								NONE, i, j, TicTacToe::TicTacToeAction(k, l)));
			}
	}
	else {
		const auto& cell = board[lastRow][lastCol];
		if (cell.isTerminal())
			return {};
		assert(!cell.isTerminal());
		for (int i = 0; i < BOARD_SIZE; ++i)
			for (int j = 0; j < BOARD_SIZE; ++j)
				if (cell.isEmpty(i, j))
					validActions.push_back(std::mksh<UltimateTicTacToeAction>(
						NONE, lastRow, lastCol, TicTacToe::TicTacToeAction(i, j)));
	}

	return validActions;
}

bool UltimateTicTacToe::isValid(const sp<Action>& act) const {
	const auto& action = std::dynamic_pointer_cast<UltimateTicTacToeAction>(act);
	assert(action);
	return isLegal(action);
}

bool UltimateTicTacToe::didWin(AgentID id) const {
	return id == getWinner();
}

up<State> UltimateTicTacToe::clone() {
	return up<State>(new UltimateTicTacToe(*this));
}

AgentID UltimateTicTacToe::getWinner() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (isRowDone(i))
			return board[i][0].getWinner();
		if (isColDone(i))
			return board[0][i].getWinner();
	}
	if (isDiag1Done())
		return board[0][0].getWinner();
	if (isDiag2Done())
		return board[0][BOARD_SIZE - 1].getWinner();
	assert(isAllTerminal());
	return NONE; 
}

std::ostream& UltimateTicTacToe::print(std::ostream& out) const {
	for (int i = 0; i < BOARD_SIZE; ++i) {
		printLineSep(out);
		for (int row = 0; row < BOARD_SIZE; ++row) {
			for (int j = 0; j < BOARD_SIZE; ++j) {
				out << "| ";
				board[i][j].printLineSep(out);
			}
			out << "|\n";
			for (int j = 0; j < BOARD_SIZE; ++j) {
				out << "| ";
				board[i][j].printRow(out, row);
			}
			out << "|\n";
		}
		for (int j = 0; j < BOARD_SIZE; ++j) {
			out << "| ";
			board[i][j].printLineSep(out);
		}
		out << "|\n";
	}
	printLineSep(out);
	return out;
}

void UltimateTicTacToe::printLineSep(std::ostream& out) const {
	std::string sep((4 * BOARD_SIZE + 1) + 2, '-');
	for (int j = 0; j < BOARD_SIZE; ++j)
		out << '+' << sep;
	out << "+\n";
}

std::string UltimateTicTacToe::getWinnerName() const {
	PROFILE_FUNCTION();

	assert(isTerminal());
	const auto winner = getWinner();
	switch (winner) {
		case AGENT1:
			return "AGENT1 / Player X";
		case AGENT2:
			return "AGENT2 / Player O";
		case NONE:
			return "";
	}
	assert(false);
}

bool UltimateTicTacToeAction::equals(const sp<Action>& o) const {
	auto oo = std::dynamic_pointer_cast<UltimateTicTacToeAction>(o);
	assert(oo);
	return row == oo->row && col == oo->col &&
		action.row == oo->action.row && action.col == oo->action.col;
}
