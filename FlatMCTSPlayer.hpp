#ifndef FLAT_MCTS_PLAYER_HPP
#define FLAT_MCTS_PLAYER_HPP

#include "Player.hpp"
#include "Common.hpp"
#include "Action.hpp"

#include <vector>

class FlatMCTSPlayer {
public:
	FlatMCTSPlayer(const int numberOfIters = 100);
	sp<Action> getAction(const up<State>& state);
	
private:
	struct ActionStats {
		int winCount = 0;
		int total = 0;
	};

	const int numberOfIters;
	std::vector<ActionStats> stats;

	sp<Action> chooseAction(const sp<State>& state,
			const std::vector<sp<Action>>& actions);
};

#endif /* MCTS_PLAYER_HPP */
