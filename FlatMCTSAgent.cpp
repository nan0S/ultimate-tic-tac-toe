#include "FlatMCTSAgent.hpp"

#include "Common.hpp"

#include <cassert>
#include <algorithm>

using ActionStats = FlatMCTSAgent::ActionStats;
using reward_t = FlatMCTSAgent::reward_t;

FlatMCTSAgent::FlatMCTSAgent(AgentID id, const up<State>&, double limitInMs, const AgentArgs&) : 
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
		
		++simulationCount;
	}

	int bestIdx = std::max_element(stats.begin(), stats.end()) - stats.begin();
	const auto& bestAction = validActions[bestIdx];
	timer.stopCalculation();

	return bestAction;
}

bool ActionStats::operator<(const ActionStats& o) const {
	return reward * o.total < o.reward * total;
}

std::vector<KeyValue> FlatMCTSAgent::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "Flat MCTS agent.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
	};
}
