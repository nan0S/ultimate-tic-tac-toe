#include "UltimateTicTacToe.hpp"

#include <algorithm>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;

UltimateTicTacToeAction::UltimateTicTacToeAction(int row, int col,
		const TicTacToe::TicTacToeAction& action) :
	row(row), col(col), action(action) {
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
	if (winner == TicTacToe::NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[row][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isColDone(int col) const {
	PROFILE_FUNCTION();

	const auto winner = board[0][col].getWinner();
	if (winner == TicTacToe::NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][col].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag1Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][0].getWinner();
	if (winner == TicTacToe::NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag2Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][BOARD_SIZE - 1].getWinner();
	if (winner == TicTacToe::NONE)
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
	turn = turn == TicTacToe::PLAYER1 ? TicTacToe::PLAYER2 : TicTacToe::PLAYER1;
}

bool UltimateTicTacToe::isLegal(const sp<UltimateTicTacToeAction>& action) const {
	PROFILE_FUNCTION();

	return isInRange(action->row) &&
		  isInRange(action->col) &&
		  board[action->row][action->col].isLegal(action->action);
}

bool UltimateTicTacToe::isInRange(int idx) const {
	PROFILE_FUNCTION();

	return 0 <= idx && idx < BOARD_SIZE;
}

std::vector<sp<Action>> UltimateTicTacToe::getValidActions() {
	PROFILE_FUNCTION();

	std::vector<sp<Action>> validActions;
	for (int i = 0; i < BOARD_SIZE; ++i)
		for (int j = 0; j < BOARD_SIZE; ++j) {
			const auto& cell = board[i][j];
			if (cell.isTerminal())
				continue;
			for (int k = 0; k < BOARD_SIZE; ++k)
				for (int l = 0; l < BOARD_SIZE; ++l)
					if (cell.isEmpty(k, l))
						validActions.push_back(std::mksh<UltimateTicTacToeAction>(
							i, j, TicTacToe::TicTacToeAction(k, l)));
		}
	return validActions;
}

up<State> UltimateTicTacToe::clone() {
	return up<State>(new UltimateTicTacToe(*this));
}

TicTacToe::PlayerTurn UltimateTicTacToe::getWinner() const {
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
	return TicTacToe::NONE; 
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
		case TicTacToe::PLAYER1:
			return "Player X";
		case TicTacToe::PLAYER2:
			return "Player O";
		case TicTacToe::NONE:
			return "";
	}
	assert(false);
}
