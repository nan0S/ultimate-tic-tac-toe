#include "MCTSAgentWithRAVE.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithRAVE::param_t;
using reward_t = MCTSAgentWithRAVE::reward_t;
using MCTSNodeBase = MCTSAgentWithRAVE::MCTSNodeBase;

MCTSAgentWithRAVE::MCTSAgentWithRAVE(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs),
	KFactor(getOrDefault(args, "KFactor", 3)),
	maxActionCount(initialState->getActionCount()) {

}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState),  {

}

sp<MCTSNodeBase> MCTSAgentWithRAVE::expand(const sp<MCTSNodeBase>& node) {
	int expandIdx = expandGetIdx(node);
	assert(expandIdx == int(node->children.size() - 1));
	assert(expandIdx == node->nextActionToResolveIdx - 1);

	treeActionHistoryIdx.emplace_back(node->actions[expandIdx]->getIdx());
	// actionHistory.emplace_back(node->state->getTurn(), node->actions[expandIdx]->getIdx());
	return node->children[expandIdx];
}

sp<MCTSNodeBase> MCTSAgentWithRAVE::select(const sp<MCTSNodeBase>& node) {
	int selectIdx = selectGetIdx(node);
	assert(selectIdx < int(node->children.size()));
	assert(selectIdx < int(node->actions.size()));

	treeActionHistoryIdx.emplace_back(node->actions[selectIdx]->getIdx());
	// actionHistory.emplace_back(node->state->getTurn(), node->actions[selectIdx]->getIdx());
	return node->children[selectIdx];
}

param_t MCTSAgentWithRAVE::eval(const sp<MCTSNodeBase>& node) {
	const auto& v = std::dynamic_pointer_cast<MCTSNode>(node);
	const auto& p = std::dynamic_pointer_cast<MCTSNode>(node->parent.lock());
	assert(v);
	assert(p);

}

sp<MCTSNodeBase> MCTSAgentWithRAVE::MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)) {

}

void MCTSAgentWithRAVE::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();
	defaultPolicyLength = 0;

	while (!state->isTerminal()) {
		auto actions = state->getValidActions(); 
		// const auto action = getActionWithDefaultPolicy(state);
		const auto& action = Random::choice(actions);
		simActionHistoryIdx.emplace_back(action->getIdx());
		state->apply(action);
		++defaultPolicyLength;
	}

	for (int i = 0; i < maxAgentCount; ++i)
		agentRewards[i] = state->getReward(AgentID(i));
}

void MCTSAgentWithRAVE::backup(sp<MCTSNodeBase> node) {
	int timesTreeAscended = 0;
	auto myID = getID();
	auto myReward = agentRewards[myID];

	while (node) {
		node->addReward(myReward, getID());
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	RAVEPolicy();
}

void MCTSAgentWithRAVE::RAVEPolicy() {

}

std::vector<KeyValue> MCTSAgentWithRAVE::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with RAVE selection and random policy simulation simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "K Factor in RAVE policy", std::to_string(KFactor) }
	};
}
