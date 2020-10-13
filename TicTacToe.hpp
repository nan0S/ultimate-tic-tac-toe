#ifndef TICTACTOE_HPP
#define TICTACTOE_HPP

#include <iostream>
#include <vector>

class TicTacToe {
public:
	TicTacToe();

	struct TicTacToeAction {
		int row, col;
		TicTacToeAction(int row, int col);
	};

	enum PlayerTurn { NONE = -1, PLAYER1, PLAYER2 };

	bool isTerminal() const;
	void apply(PlayerTurn turn, const TicTacToeAction& action);

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int row) const;

	PlayerTurn getWinner() const;

	bool isLegal(const TicTacToeAction& action) const;
	bool isEmpty(int i, int j) const;

private:
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board Size has to be positive");

	PlayerTurn board[BOARD_SIZE][BOARD_SIZE];
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
