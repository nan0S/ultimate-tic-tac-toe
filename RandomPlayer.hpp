#ifndef RANDOM_PLAYER_HPP
#define RANDOM_PLAYER_HPP

#include "Player.hpp"

class RandomPlayer : public Player {
public:
	sp<Action> getAction(const sp<State>& state) override;
};

#endif /* RANDOM_PLAYER_HPP */
