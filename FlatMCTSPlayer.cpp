#include "FlatMCTSPlayer.hpp"

#include "Common.hpp"

#include <cassert>
#include <algorithm>

FlatMCTSPlayer::FlatMCTSPlayer(int numberOfIters) : numberOfIters(numberOfIters) {

}

sp<Action> FlatMCTSPlayer::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());

	int actionsNum = static_cast<int>(validActions.size());
	stats.resize(actionsNum);
	std::fill(stats.begin(), stats.end(), ActionStats());

	for (int i = 0; i < numberOfIters; ++i) {
		int actionIdx = Random::rand(actionsNum);
		State nState = state->applyCopy(validActions[actionIdx]);
	}
}

sp<Action> FlatMCTSPlayer::chooseAction(const sp<State>& state,
		const std::vector<sp<Action>>& actions) {

		
}

