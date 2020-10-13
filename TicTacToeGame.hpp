#ifndef ULTIMATE_TICTACTOE_HPP
#define ULTIMATE_TICTACTOE_HPP

#include "TicTacToe.hpp"

class UltimateTicTacToe : public State {
public:
	UltimateTicTacToe();

	bool isTerminal() override;
	void apply(const sp<Action>& action) override;
	std::vector<sp<Action>> getValidActions() override;

	enum PlayerTurn { NONE = -1, PLAYER1, PLAYER2 };
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board Size has to be positive");

	std::ostream& print(std::ostream& out) const override;
	void printLineSep(std::ostream& out) const;
	inline char getCharAt(int row, int col) const;

	std::string getWinnerName() const override;

private:
	PlayerTurn turn = PLAYER1;
	TicTacToe board[BOARD_SIZE][BOARD_SIZE];
	int freeCells;

	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	inline bool isEmpty(int i, int j) const;
	inline bool isLegal(const sp<TicTacToeAction>& action) const;
	inline bool inRange(int idx) const;
	
	PlayerTurn getWinner() const;
};

#endif /* ULTIMATE_TICTACTOE_HPP */
