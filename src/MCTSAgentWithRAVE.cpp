#include "MCTSAgentWithRAVE.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithRAVE::param_t;
using reward_t = MCTSAgentWithRAVE::reward_t;
using MCTSNodeBase = MCTSAgentWithRAVE::MCTSNodeBase;

MCTSAgentWithRAVE::MCTSAgentWithRAVE(AgentID id, double calcLimitInMs,
		const up<State>& initialState, const AgentArgs& args) :
	MCTSAgentBase(id, calcLimitInMs, std::mku<MCTSNode>(initialState)),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)),
	KFactor(getOrDefault(args, "KFactor", 50.0)),
	maxActionCount(initialState->getActionCount()) {

}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState), actionsStats(this->state->getActionCount()) {

}

sp<MCTSNodeBase> MCTSAgentWithRAVE::expand(const sp<MCTSNodeBase>& node) {
	int expandIdx = expandGetIdx(node);
	assert(expandIdx == int(node->children.size() - 1));
	assert(expandIdx == node->nextActionToResolveIdx - 1);

	actionHistory.emplace_back(node->actions[expandIdx]->getIdx());
	return node->children[expandIdx];
}

sp<MCTSNodeBase> MCTSAgentWithRAVE::select(const sp<MCTSNodeBase>& node) {
	int selectIdx = selectGetIdx(node);
	assert(selectIdx < int(node->children.size()));
	assert(selectIdx < int(node->actions.size()));

	actionHistory.emplace_back(node->actions[selectIdx]->getIdx());
	return node->children[selectIdx];
}

param_t MCTSAgentWithRAVE::eval(const sp<MCTSNodeBase>& node, const sp<Action>& action) {
	const auto& v = std::dynamic_pointer_cast<MCTSNode>(node);
	const auto& p = std::dynamic_pointer_cast<MCTSNode>(node->parent.lock());
	assert(v);
	assert(p);

	int actionIdx = action->getIdx();
	assert(actionIdx < int(p->actionsStats.size()));

	param_t exploitationFactor = param_t(v->stats.score) / v->stats.visits;
	param_t explorationFactor = std::sqrt(2.0 * std::log(p->stats.visits) / v->stats.visits);
	param_t qValue = exploitationFactor + exploreFactor * explorationFactor;

	param_t qAMAF = param_t(p->actionsStats[actionIdx].reward) / p->actionsStats[actionIdx].visits;
	param_t beta = std::sqrt(KFactor / (3 * p->stats.visits + KFactor));

	return (1 - beta) * qValue + beta * qAMAF;
}

sp<MCTSNodeBase> MCTSAgentWithRAVE::MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)), actionsStats(this->state->getActionCount()) {

}

void MCTSAgentWithRAVE::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();
	defaultPolicyLength = 0;

	while (!state->isTerminal()) {
		auto actions = state->getValidActions(); 
		const auto& action = Random::choice(actions);
		actionHistory.emplace_back(action->getIdx());
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

	int actionHistoryCount = int(actionHistory.size());
	int actionBeginIdx = actionHistoryCount - defaultPolicyLength;
	auto v = std::dynamic_pointer_cast<MCTSNode>(node);

	while (v) {
		assert(actionBeginIdx >= 0);
		auto currentReward = agentRewards[v->state->getTurn()];
		for (int i = actionBeginIdx; i < actionHistoryCount; i += 2) {
			auto& stats = v->actionsStats[actionHistory[i]];
			++stats.visits;
			stats.reward += currentReward;
		}

		v->addReward(myReward, getID());
		v = std::dynamic_pointer_cast<MCTSNode>(v->parent.lock());
		assert(node);

		++timesTreeAscended;
		--actionBeginIdx;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	assert(actionBeginIdx == -1);
	actionHistory.clear();
}

std::vector<KeyValue> MCTSAgentWithRAVE::getDesc(double avgSimulationCount) const {
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with RAVE selection and random policy simulation simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(avgSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "K Factor in RAVE policy", std::to_string(KFactor) }
	};
}
