#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Common.hpp"
#include "State.hpp"
#include "Action.hpp"

class Player {
public:
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual ~Player() = default;
};

#endif /* PLAYER_HPP */
