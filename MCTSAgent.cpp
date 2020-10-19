#include "MCTSAgent.hpp"

#include <cassert>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;
using MCTSNodeBase = MCTSAgent::MCTSNodeBase;

MCTSAgent::MCTSAgent(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args) {

}

MCTSAgent::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgent::treePolicy() {
	auto currentNode = root;
	descended = 0;

	while (!currentNode->isTerminal()) {
		++descended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = currentNode->selectChild(exploreFactor);
	}

	return currentNode;
}

MCTSAgent::MCTSNode::MCTSNode(up<State>&& initialState)
	: MCTSNodeBase(std::move(initialState)) {

}

reward_t MCTSAgent::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();

     while (!state->isTerminal()) {
		auto actions = state->getValidActions();
		const auto& action = Random::choice(actions);
		state->apply(action);
	}

	return state->getReward(getID());
}

void MCTSAgent::backup(sp<MCTSNodeBase> node, reward_t delta) {
	int ascended = 0;

	while (node) {
		node->addReward(delta, getID());
		node = node->parent.lock();
		++ascended;
	}

	assert(descended + 1 == ascended);
}

sp<MCTSNodeBase> MCTSAgent::MCTSNode::getChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
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
