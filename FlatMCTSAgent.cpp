#include "FlatMCTSAgent.hpp"

#include "Common.hpp"

#include <cassert>
#include <algorithm>

using ActionStats = FlatMCTSAgent::ActionStats;

FlatMCTSAgent::FlatMCTSAgent(AgentID id, int numberOfIters) : 
	Agent(id), numberOfIters(numberOfIters) {

}

sp<Action> FlatMCTSAgent::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	std::shuffle(validActions.begin(), validActions.end(), Random::rng);

	int actionsNum = static_cast<int>(validActions.size());
	stats.resize(actionsNum);
	std::fill(stats.begin(), stats.end(), ActionStats());

	int perm[actionsNum];
	std::iota(perm, perm + actionsNum, 0);
	
	for (int i = 0; i < numberOfIters; ++i) {
		std::shuffle(perm, perm + actionsNum, Random::rng);
		int randActionIdx = perm[0];
		int consActionsIdx = 1;
		auto nState = state->applyCopy(validActions[randActionIdx]);

		while (!nState->isTerminal()) {
			// we assume that in initialState we get all valid actions which will become invalid
			// after some actions are done, but no other will come
			// it's not in general but in UltimateTicTacToe it's true
			while (!nState->isValid(validActions[perm[consActionsIdx]])) {
				++consActionsIdx;
				assert(consActionsIdx < actionsNum);
			}
			const auto& action = validActions[perm[consActionsIdx]];
			nState->apply(action);
		}

		++stats[randActionIdx].total;
		if (nState->didWin(getID()))
			++stats[randActionIdx].winCount;
	}

	int bestIdx = std::max_element(stats.begin(), stats.end()) - stats.begin();
	return validActions[bestIdx];
}

bool ActionStats::operator<(const ActionStats& o) const {
	return 1ll * winCount * o.total < 1ll * o.winCount * total;
}

std::map<std::string, std::string> FlatMCTSAgent::getDesc() const {
	return { { "Flat MCTS agent.", "" },
		{ "Number of MCTS iterations", std::to_string(numberOfIters) }
	};
}
