#ifndef MCTS_PLAYER_HPP
#define MCTS_PLAYER_HPP

#include "Player.hpp"
#include "Common.hpp"
#include "Action.hpp"

class MCTSPlayer {
public:
	sp<Action> getAction(const sp<State>& state);
};

#endif /* MCTS_PLAYER_HPP */
