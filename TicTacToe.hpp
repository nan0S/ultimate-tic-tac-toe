#ifndef TICTACTOE_HPP
#define TICTACTOE_HPP

#include <iostream>
#include <vector>

#include "Agent.hpp"

class TicTacToe {
public:
	TicTacToe();

	struct TicTacToeAction {
		int row, col;
		TicTacToeAction(int row, int col);
	};

	bool isTerminal() const;
	void apply(AgentID turn, const TicTacToeAction& action);

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int row) const;

	AgentID getWinner() const;

	bool isLegal(const TicTacToeAction& action) const;
	bool isEmpty(int i, int j) const;

	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board Size has to be positive");
	AgentID board[BOARD_SIZE][BOARD_SIZE];
private:

	int emptyCells = BOARD_SIZE * BOARD_SIZE;

	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	inline bool isInRange(int idx) const;

	inline char getCharAt(int row, int col) const;
	inline char getCharAtTerminal(int row, int col) const;
	inline char convertSymbolAt(int row, int col) const;

	inline bool isOnDiag1(int row, int col) const;
	inline bool isOnDiag2(int row, int col) const;
	inline bool isOnSide(int row, int col) const;
	inline bool isOnTop(int row, int col) const;
};

#endif /* TICTACTOE_HPP */
