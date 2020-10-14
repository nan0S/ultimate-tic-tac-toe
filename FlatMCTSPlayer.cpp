#include "FlatMCTSPlayer.hpp"

#include "Common.hpp"

#include <cassert>
#include <algorithm>

using ActionStats = FlatMCTSPlayer::ActionStats;

FlatMCTSPlayer::FlatMCTSPlayer(int numberOfIters) : numberOfIters(numberOfIters) {

}

sp<Action> FlatMCTSPlayer::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());

	int actionsNum = static_cast<int>(validActions.size());
	stats.resize(actionsNum);
	std::fill(stats.begin(), stats.end(), ActionStats());

	int perm[actionsNum];
	std::iota(perm, perm + actionsNum, 0);
	
	for (int i = 0; i < numberOfIters; ++i) {
          int randActionIdx = Random::rand(actionsNum);

          auto nState = state->applyCopy(validActions[randActionIdx]);
		nState->record();

		int consActionsIdx = 0;
		std::shuffle(perm, perm + actionsNum, Random::rng);
		
		while (!nState->isTerminal()) {
			while (!nState->isValid(validActions[perm[consActionsIdx]])) {
				++consActionsIdx;
				assert(consActionsIdx < actionsNum);
			}
			const auto& action = validActions[perm[consActionsIdx]];
			nState->apply(action);
		}

		++stats[randActionIdx].total;
		if (nState->didWon())
			++stats[randActionIdx].winCount;
	}

	int bestIdx = std::max_element(stats.begin(), stats.end()) - stats.begin();
	return validActions[bestIdx];
}

bool ActionStats::operator<(const ActionStats& o) const {
	return 1ll * winCount * o.total < 1ll * o.winCount * total;
}

std::map<std::string, std::string> FlatMCTSPlayer::getDesc() const {
	return { { "Flat MCTS player.", "" },
		{ "Number of MCTS iterations", std::to_string(numberOfIters) }
	};
}
