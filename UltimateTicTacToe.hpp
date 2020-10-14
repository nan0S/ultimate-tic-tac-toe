#ifndef ULTIMATE_TICTACTOE_HPP
#define ULTIMATE_TICTACTOE_HPP

#include "Common.hpp"
#include "Action.hpp"
#include "State.hpp"
#include "TicTacToe.hpp"

class UltimateTicTacToe : public State {
public:
	struct UltimateTicTacToeAction : public Action {
		int row, col;
		TicTacToe::TicTacToeAction action;
		UltimateTicTacToeAction(int row, int col, const TicTacToe::TicTacToeAction& action);
	};

	bool isTerminal() const override;
	void apply(const sp<Action>& act) override;

	std::vector<sp<Action>> getValidActions() override;
	bool isValid(const sp<Action>& act) override;

	void record() override;
	bool didWon() override;
	up<State> clone() override;

	std::ostream& print(std::ostream& out) const override;
	std::string getWinnerName() const override;

	bool isLegal(const sp<UltimateTicTacToeAction>& action) const;
	
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board size has to be positive");

private:
	bool recording = false;
	TicTacToe::PlayerTurn recordedTurn;

	TicTacToe::PlayerTurn turn = TicTacToe::PLAYER1;
	TicTacToe board[BOARD_SIZE][BOARD_SIZE];

	bool isAllTerminal() const;
	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	bool isInRange(int idx) const;

	TicTacToe::PlayerTurn getWinner() const;

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int i) const;
};

#endif /* ULTIMATE_TICTACTOE_HPP */
