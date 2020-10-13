#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Common.hpp"
#include "State.hpp"
#include "Action.hpp"

class Player {
public:
	virtual sp<Action> getAction(const sp<State>& state) = 0;
};

#endif /* PLAYER_HPP */
