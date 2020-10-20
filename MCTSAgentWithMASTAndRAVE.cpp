#include "MCTSAgentWithMASTAndRAVE.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithMASTAndRAVE::param_t;
using reward_t = MCTSAgentWithMASTAndRAVE::reward_t;
using MCTSNodeBase = MCTSAgentWithMASTAndRAVE::MCTSNodeBase;

MCTSAgentWithMASTAndRAVE::MCTSAgentWithMASTAndRAVE(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)),
	epsilon(getOrDefault(args, "epsilon", 0.8)),
	decayFactor(getOrDefault(args, "decayFactor", 0.6)),
	KFactor(getOrDefault(args, "KFactor", 50.0)),
	maxActionCount(initialState->getActionCount()),
	actionsStats(maxAgentCount) {

	std::fill(actionsStats.begin(), actionsStats.end(),
			std::vector<MASTAndRAVEActionStats>(maxActionCount));
}

MCTSAgentWithMASTAndRAVE::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState), actionsStats(initialState->getActionCount()) {

}

sp<MCTSNodeBase> MCTSAgentWithMASTAndRAVE::expand(const sp<MCTSNodeBase>& node) {
	int expandIdx = expandGetIdx(node);
	assert(expandIdx == int(node->children.size() - 1));
	assert(expandIdx == node->nextActionToResolveIdx - 1);

	actionHistory.emplace_back(node->state->getTurn(), node->actions[expandIdx]->getIdx());
	return node->children[expandIdx];
}

sp<MCTSNodeBase> MCTSAgentWithMASTAndRAVE::select(const sp<MCTSNodeBase>& node) {
	int selectIdx = selectGetIdx(node);
	assert(selectIdx < int(node->children.size()));
	assert(selectIdx < int(node->actions.size()));

	actionHistory.emplace_back(node->state->getTurn(), node->actions[selectIdx]->getIdx());
	return node->children[selectIdx];
}

param_t MCTSAgentWithMASTAndRAVE::eval(const sp<MCTSNodeBase>& node, const sp<Action>& action) {
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

sp<MCTSNodeBase> MCTSAgentWithMASTAndRAVE::MCTSNode::makeChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithMASTAndRAVE::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)), actionsStats(this->state->getActionCount()) {

}

void MCTSAgentWithMASTAndRAVE::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
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

sp<Action> MCTSAgentWithMASTAndRAVE::getActionWithDefaultPolicy(const up<State>& state) {
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

void MCTSAgentWithMASTAndRAVE::backup(sp<MCTSNodeBase> node) {
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
			int actionIdx = actionHistory[i].second;
			auto& stats = v->actionsStats[actionIdx];
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

	MASTPolicy();
}

void MCTSAgentWithMASTAndRAVE::MASTPolicy() {
	assert(defaultPolicyLength + timesTreeDescended == int(actionHistory.size()));
	for (const auto& [agentID, actionIdx] : actionHistory)
		updateActionStat(agentID, actionIdx);
	actionHistory.clear();
}

void MCTSAgentWithMASTAndRAVE::updateActionStat(AgentID id, int actionIdx) {
	assert(id < int(actionsStats.size()));
	assert(actionIdx < int(actionsStats[id].size()));

	auto& statToUpdate = actionsStats[id][actionIdx];
	statToUpdate.score += agentRewards[id];
	++statToUpdate.times;
}

void MCTSAgentWithMASTAndRAVE::postWork() {
	for (auto& v : actionsStats)
		for (auto& x : v)
			x.score *= decayFactor;
}

std::vector<KeyValue> MCTSAgentWithMASTAndRAVE::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with RAVE selection policy and MAST epsilon-greedy simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
		{ "Epsilon constant (E) in MAST default policy", std::to_string(epsilon) },
		{ "Decay factor (gamma) in MAST global action table", std::to_string(decayFactor) },
		{ "K Factor in RAVE policy", std::to_string(KFactor) }
	};
}
