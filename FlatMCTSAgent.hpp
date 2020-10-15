#ifndef FLAT_MCTS_PLAYER_HPP
#define FLAT_MCTS_PLAYER_HPP

#include "Player.hpp"
#include "Common.hpp"
#include "Action.hpp"

#include <vector>

class FlatMCTSPlayer : public Player {
public:
	FlatMCTSPlayer(const int numberOfIters = 200);
	sp<Action> getAction(const up<State>& state);
	
	struct ActionStats {
		int winCount = 0;
		int total = 0;
		bool operator<(const ActionStats& o) const;
	};

	std::map<std::string, std::string> getDesc() const override;

private:
	const int numberOfIters;
	std::vector<ActionStats> stats;
};

#endif /* MCTS_PLAYER_HPP */
