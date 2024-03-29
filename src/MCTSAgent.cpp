#include "MCTSAgent.hpp"

#include <cassert>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;
using MCTSNodeBase = MCTSAgent::MCTSNodeBase;

MCTSAgent::MCTSAgent(AgentID id, double calcLimitInMs,
		const up<State>& initialState, const AgentArgs& args) :
	MCTSAgentBase(id, calcLimitInMs, std::mku<MCTSNode>(initialState)),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)) {

}

MCTSAgent::MCTSNode::MCTSNode(const up<State>& initialState) : MCTSNodeBase(initialState) {

}

param_t MCTSAgent::eval(const sp<MCTSNodeBase>& node, const sp<Action>&) {
	const auto& v = std::dynamic_pointer_cast<MCTSNode>(node);
	const auto& p = std::dynamic_pointer_cast<MCTSNode>(node->parent.lock());
	assert(v);
	assert(p);
	param_t exploitationFactor = param_t(v->stats.score) / v->stats.visits;
	param_t explorationFactor = std::sqrt(2.0 * std::log(p->stats.visits) / v->stats.visits);
	return exploitationFactor + exploreFactor * explorationFactor;
}

void MCTSAgent::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();

     while (!state->isTerminal()) {
		auto actions = state->getValidActions();
		const auto& action = Random::choice(actions);
		state->apply(action);
	}

	for (int i = 0; i < maxAgentCount; ++i)
		agentRewards[i] = state->getReward(AgentID(i));
}

void MCTSAgent::backup(sp<MCTSNodeBase> node) {
	int timesTreeAscended = 0;
	auto myReward = agentRewards[getID()];
	auto myID = getID();

	while (node) {
		node->addReward(myReward, myID);
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
}

sp<MCTSNodeBase> MCTSAgent::MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgent::MCTSNode::MCTSNode(up<State>&& initialState) : MCTSNodeBase(std::move(initialState)) {

}

std::vector<KeyValue> MCTSAgent::getDesc(double avgSimulationCount) const {
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(avgSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
	};
}
