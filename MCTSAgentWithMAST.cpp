#include "MCTSAgentWithMAST.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithMAST::param_t;
using reward_t = MCTSAgentWithMAST::reward_t;
using MCTSNodeBase = MCTSAgentWithMAST::MCTSNodeBase;

MCTSAgentWithMAST::MCTSAgentWithMAST(AgentID id, double calcLimitInMs,
		const up<State>& initialState, const AgentArgs& args) :
	MCTSAgentBase(id, calcLimitInMs, std::mku<MCTSNode>(initialState)),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)),
	epsilon(getOrDefault(args, "epsilon", 0.8)),
	decayFactor(getOrDefault(args, "decayFactor", 0.6)),
	maxActionCount(initialState->getActionCount()),
	actionsStats(maxAgentCount) {

	std::fill(actionsStats.begin(), actionsStats.end(),
			std::vector<MASTActionStats>(maxActionCount));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgentWithMAST::expand(const sp<MCTSNodeBase>& node) {
	int expandIdx = expandGetIdx(node);
	assert(expandIdx == int(node->children.size() - 1));
	assert(expandIdx == node->nextActionToResolveIdx - 1);

	actionHistory.emplace_back(node->state->getTurn(), node->actions[expandIdx]->getIdx());
	return node->children[expandIdx];
}

sp<MCTSNodeBase> MCTSAgentWithMAST::select(const sp<MCTSNodeBase>& node) {
	int selectIdx = selectGetIdx(node);
	assert(selectIdx < int(node->children.size()));
	assert(selectIdx < int(node->actions.size()));

	actionHistory.emplace_back(node->state->getTurn(), node->actions[selectIdx]->getIdx());
	return node->children[selectIdx];
}

param_t MCTSAgentWithMAST::eval(const sp<MCTSNodeBase>& node, const sp<Action>&) {
	const auto& v = std::dynamic_pointer_cast<MCTSNode>(node);
	const auto& p = std::dynamic_pointer_cast<MCTSNode>(node->parent.lock());
	assert(v);
	assert(p);
	param_t exploitationFactor = param_t(v->stats.score) / v->stats.visits;
	param_t explorationFactor = std::sqrt(2.0 * std::log(p->stats.visits) / v->stats.visits);
	return exploitationFactor + exploreFactor * explorationFactor;
}

sp<MCTSNodeBase> MCTSAgentWithMAST::MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)) {

}

void MCTSAgentWithMAST::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();
	defaultPolicyLength = 0;

	while (!state->isTerminal()) {
		const auto action = getActionWithDefaultPolicy(state);
		actionHistory.emplace_back(state->getTurn(), action->getIdx());
		state->apply(action);
		++defaultPolicyLength;
	}

	for (int i = 0; i < maxAgentCount; ++i)
		agentRewards[i] = state->getReward(AgentID(i));
}

sp<Action> MCTSAgentWithMAST::getActionWithDefaultPolicy(const up<State>& state) {
	auto actions = state->getValidActions();
	assert(!actions.empty());

	if (Random::rand(1.0) <= epsilon)
		return Random::choice(actions);

	return *std::max_element(actions.begin(), actions.end(),
			[&state, this](const sp<Action>& a1, const sp<Action>& a2){
		const auto& s1 = actionsStats[state->getTurn()][a1->getIdx()];
		const auto& s2 = actionsStats[state->getTurn()][a2->getIdx()];
		return s1.score * s2.times < s2.score * s1.times;
	});
}

void MCTSAgentWithMAST::backup(sp<MCTSNodeBase> node) {
	int timesTreeAscended = 0;
	auto myID = getID();
	auto myReward = agentRewards[myID];

	while (node) {
		node->addReward(myReward, getID());
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	MASTPolicy();
}

void MCTSAgentWithMAST::MASTPolicy() {
	assert(defaultPolicyLength + timesTreeDescended == int(actionHistory.size()));
	for (const auto& [agentID, actionIdx] : actionHistory)
		updateActionStat(agentID, actionIdx);
	actionHistory.clear();
}

void MCTSAgentWithMAST::updateActionStat(AgentID id, int actionIdx) {
	assert(id < int(actionsStats.size()));
	assert(actionIdx < int(actionsStats[id].size()));

	auto& statToUpdate = actionsStats[id][actionIdx];
	statToUpdate.score += agentRewards[id];
	++statToUpdate.times;
}

void MCTSAgentWithMAST::postWork() {
	for (auto& v : actionsStats)
		for (auto& x : v)
			x.score *= decayFactor;
}

std::vector<KeyValue> MCTSAgentWithMAST::getDesc(double avgSimulationCount) const {
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with UCT selection and MAST policy with epsilon-greedy simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(avgSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
		{ "Epsilon constant (E) in MAST default policy", std::to_string(epsilon) },
		{ "Decay factor (gamma) in MAST global action table", std::to_string(decayFactor) }
	};
}
