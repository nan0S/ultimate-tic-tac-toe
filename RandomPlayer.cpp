#include "RandomPlayer.hpp"

#include <cassert>

sp<Action> RandomPlayer::getAction(const sp<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}
