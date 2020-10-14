#ifndef MCTS_PLAYER_HPP
#define MCTS_PLAYER_HPP

#include "Player.hpp"

class MCTSPlayer : public Player {
public:
	MCTSPlayer(const up<State>& initialState, int numberOfIters = 100);
	sp<Action> getAction(const up<State>& state) override;
	void recordAction(const sp<Action>& action) override;
	std::map<std::string, std::string> getDesc() const override;

private:
	int numberOfIters;

	class MCTSNode {
	public:
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		inline bool isTerminal() const;
	private:
		up<State> state;
		std::vector<sp<MCTSNode>> children;
		std::vector<sp<Action>> actions;
		int nextActionToResolveIdx = 0;
	};

	sp<MCTSNode> root;

	sp<MCTSNode> treePolicy();
};

#endif /* MCTS_PLAYER_HPP */

/*
 * node reprezentuje stan
 * node zawiera statystyki: win, total
 * node zawiera liste expanded dzieci
 * node zawiera liste zszufflowanych akcji poprawnych w stanie, ktory reprezentuje
 * node zawiera stan
 * node zawiera iterator/index do nastepnej akcji ktora uzyjemy w expand
*/
