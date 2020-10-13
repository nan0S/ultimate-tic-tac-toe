#include "TicTacToe.hpp"

#include "Common.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>

#undef PROFILING
#define PROFILING 0

using TicTacToeAction = TicTacToe::TicTacToeAction;

TicTacToe::TicTacToe() {
	PROFILE_FUNCTION();
	for (int i = 0; i < BOARD_SIZE; ++i)
		std::fill(board[i], board[i] + BOARD_SIZE, NONE);
}

TicTacToe::TicTacToeAction::TicTacToeAction(int row, int col) : row(row), col(col) {

}

bool TicTacToe::isTerminal() const {
	PROFILE_FUNCTION();

	if (emptyCells == 0)
		return true;
	for (int i = 0; i < BOARD_SIZE; ++i)
		if (isRowDone(i) || isColDone(i))
			return true;
	return isDiag1Done() || isDiag2Done();
}

void TicTacToe::apply(PlayerTurn turn, const TicTacToeAction& action) {
	PROFILE_FUNCTION();

	assert(isLegal(action));
	board[action.row][action.col] = turn;
	--emptyCells;
}

bool TicTacToe::isRowDone(int row) const {
	PROFILE_FUNCTION();

	PlayerTurn val =  board[row][0];
	return val != NONE && 
		std::all_of(board[row], board[row] + BOARD_SIZE,
				  [&val](const PlayerTurn& p){ return p == val; });
}

bool TicTacToe::isColDone(int col) const {
	PROFILE_FUNCTION();

	PlayerTurn val =  board[0][col];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][col] != val)
			return false;
	return true;
}

bool TicTacToe::isDiag1Done() const {
	PROFILE_FUNCTION();

	PlayerTurn val = board[0][0];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][i] != val)
			return false;
	return true;
}

bool TicTacToe::isDiag2Done() const {
	PROFILE_FUNCTION();

	PlayerTurn val = board[0][BOARD_SIZE - 1];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][BOARD_SIZE - 1 - i] != val)
			return false;
	return true;
}

bool TicTacToe::isEmpty(int i, int j) const {
	PROFILE_FUNCTION();

	return board[i][j] == NONE;
}

bool TicTacToe::isLegal(const TicTacToeAction& action) const {
	PROFILE_FUNCTION();

	return isInRange(action.row) && 
		  isInRange(action.col) && 
		  isEmpty(action.row, action.col);
}

bool TicTacToe::isInRange(int idx) const {
	PROFILE_FUNCTION();

	return 0 <= idx && idx < BOARD_SIZE;
}

TicTacToe::PlayerTurn TicTacToe::getWinner() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (isRowDone(i))
			return board[i][0];
		if (isColDone(i))
			return board[0][i];
	}
	if (isDiag1Done())
		return board[0][0];
	if (isDiag2Done())
		return board[0][BOARD_SIZE - 1];
	return NONE;
}

void TicTacToe::printLineSep(std::ostream& out) const {
	for (int i = 0; i < BOARD_SIZE; ++i)
		out << "+---";
	out << "+ ";
}

void TicTacToe::printRow(std::ostream& out, int row) const {
	for (int j = 0; j < BOARD_SIZE; ++j)
		out << "| " << getCharAt(row, j) << " ";
	out << "| ";
}

char TicTacToe::getCharAt(int row, int col) const {
	if (isTerminal())
		return getCharAtTerminal(row, col);
	return convertSymbolAt(row, col);

}

char TicTacToe::getCharAtTerminal(int row, int col) const {
	auto winner = getWinner();
	if (winner == PLAYER1)
		return isOnDiag1(row, col) || 
			  isOnDiag2(row, col) ?
			  'X' : ' ';
	else if (winner == PLAYER2)
		return isOnSide(row, col) ||
			  isOnTop(row, col) ?
			  'O' : ' ';
	return convertSymbolAt(row, col);
}

char TicTacToe::convertSymbolAt(int row, int col) const {
	const auto symbol = board[row][col];
	switch (symbol) {
		case PLAYER1:
			return 'X';
		case PLAYER2:
			return 'O';
		case NONE:
			return ' ';
	}
	assert(false);
}

bool TicTacToe::isOnDiag1(int row, int col) const {
	return row == col;
}

bool TicTacToe::isOnDiag2(int row, int col) const {
	return row + col == BOARD_SIZE - 1;
}

bool TicTacToe::isOnSide(int row, int col) const {
	return col == 0 || col == BOARD_SIZE - 1;
}

bool TicTacToe::isOnTop(int row, int col) const {
	return row == 0 || row == BOARD_SIZE - 1;
}
