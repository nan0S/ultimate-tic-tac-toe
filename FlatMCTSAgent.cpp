#include "FlatMCTSAgent.hpp"

#include "Common.hpp"

#include <cassert>
#include <algorithm>

using ActionStats = FlatMCTSAgent::ActionStats;
using reward_t = FlatMCTSAgent::reward_t;

FlatMCTSAgent::FlatMCTSAgent(AgentID id, double limitInMs) : 
	Agent(id),
	timer(limitInMs) {

}

sp<Action> FlatMCTSAgent::getAction(const up<State>& state) {
	timer.startCalculation();

	auto validActions = state->getValidActions();
	assert(!validActions.empty());

	int actionsNum = static_cast<int>(validActions.size());
	stats.resize(actionsNum);
	std::fill(stats.begin(), stats.end(), ActionStats());
	
	for (int i = 0; i < 100; ++i) {
		int randActionIdx = Random::rand(actionsNum);
		auto nState = state->applyCopy(validActions[randActionIdx]);

		while (!nState->isTerminal()) {
			auto actions = nState->getValidActions();
			const auto& action = Random::choice(actions);
			nState->apply(action);
		}

		++stats[randActionIdx].total;
		// stats[randActionIdx].reward += nState->getReward(getID());
		if (nState->didWin(getID()))
			++stats[randActionIdx].reward;
	}

	int bestIdx = std::max_element(stats.begin(), stats.end()) - stats.begin();
	const auto& bestAction = validActions[bestIdx];
	timer.endCalculation();

	return bestAction;
}

bool ActionStats::operator<(const ActionStats& o) const {
	// return 1ll * winCount * o.total < 1ll * o.winCount * total;
	return reward * o.total < o.reward * total;
}

std::vector<KeyValue> FlatMCTSAgent::getDesc() const {
	return { { "Flat MCTS agent.", "" },
		{ "Number of MCTS iterations", std::to_string(100) },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
	};
}
