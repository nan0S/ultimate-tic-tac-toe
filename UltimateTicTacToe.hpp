#ifndef ULTIMATE_TICTACTOE_HPP
#define ULTIMATE_TICTACTOE_HPP

#include "Common.hpp"
#include "Action.hpp"
#include "State.hpp"
#include "TicTacToe.hpp"

class UltimateTicTacToe : public State {
public:
	using reward_t = State::reward_t;

	struct UltimateTicTacToeAction : public Action {
		UltimateTicTacToeAction(const AgentID& agentID, int row, int col,
				const TicTacToe::TicTacToeAction& action);

		bool equals(const sp<Action>& o) const override;
		int getIdx() const override;

		AgentID agentID;
		int row, col;
		TicTacToe::TicTacToeAction action;
	};

	bool isTerminal() const override;
	void apply(const sp<Action>& act) override;

	constexpr int getAgentCount() const override;
	constexpr int getActionCount() const override;

	std::vector<sp<Action>> getValidActions() override;
	bool isValid(const sp<Action>& act) const override;

	up<State> clone() override;
	bool didWin(AgentID id) const override; 
	reward_t getReward(AgentID id) const override;
	AgentID getTurn() const override;

	std::ostream& print(std::ostream& out) const override;
	std::string getWinnerName() const override;

	bool isLegal(const sp<UltimateTicTacToeAction>& action) const;
	
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board size has to be positive");

private:
	TicTacToe board[BOARD_SIZE][BOARD_SIZE];
	AgentID turn = AGENT1;
	int lastRow = -1, lastCol = -1;

	bool isAllTerminal() const;
	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	bool isInRange(int idx) const;
	bool canMove(AgentID agentID) const;
	bool properBoard(int boardRow, int boardCol) const;

	AgentID getWinner() const;

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int i) const;
};

#endif /* ULTIMATE_TICTACTOE_HPP */
