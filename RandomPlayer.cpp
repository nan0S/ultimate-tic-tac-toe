#include "RandomPlayer.hpp"

#include <cassert>

sp<Action> RandomPlayer::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}

std::map<std::string, std::string> RandomPlayer::getDesc() const {
	return { {"Random player with uniform distribution.", ""} };
}
