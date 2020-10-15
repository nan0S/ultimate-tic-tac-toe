#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Common.hpp"
#include "State.hpp"
#include "Action.hpp"

#include <map>

class Player {
public:
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::map<std::string, std::string> getDesc() const;
	virtual ~Player() = default;
};

#endif /* PLAYER_HPP */
