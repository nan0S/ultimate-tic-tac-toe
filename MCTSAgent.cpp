#include "MCTSAgent.hpp"

#include <cassert>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;
using MCTSNode = MCTSAgent::MCTSNode;
using MCTSNodeBase = MCTSAgent::MCTSNodeBase;

MCTSAgent::MCTSAgent(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args) {

}

MCTSNode::MCTSNode(const up<State>& initialState) : MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgent::treePolicy() {
	auto currentNode = root;
	descended = 0;

	while (!currentNode->isTerminal()) {
		++descended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = select(currentNode);
	}

	return currentNode;
}

param_t MCTSAgent::eval(const sp<MCTSNodeBase>& node) {
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
	int ascended = 0;
	auto myReward = agentRewards[getID()];
	auto myID = getID();

	while (node) {
		node->addReward(myReward, myID);
		node = node->parent.lock();
		++ascended;
	}

	assert(descended + 1 == ascended);
}

sp<MCTSNodeBase> MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSNode::MCTSNode(up<State>&& initialState) : MCTSNodeBase(std::move(initialState)) {

}

std::vector<KeyValue> MCTSAgent::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
	};
}
