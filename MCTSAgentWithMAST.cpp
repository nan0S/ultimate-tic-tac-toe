#include "MCTSAgentWithMAST.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithMAST::param_t;
using reward_t = MCTSAgentWithMAST::reward_t;
using MCTSNodeBase = MCTSAgentWithMAST::MCTSNodeBase;


MCTSAgentWithMAST::MCTSAgentWithMAST(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args),
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

sp<MCTSNodeBase> MCTSAgentWithMAST::treePolicy() {
	auto currentNode = std::dynamic_pointer_cast<MCTSNode>(root);
	assert(currentNode);
	timesTreeDescended = 0;

	while (!currentNode->isTerminal()) {
		++timesTreeDescended;

		if (currentNode->shouldExpand()) {
			int expandIdx = expandAndGetIdx(currentNode);
			assert(expandIdx == int(currentNode->children.size() - 1));
			assert(expandIdx == currentNode->nextActionToResolveIdx - 1);

			actionHistory.emplace_back(currentNode->state->getTurn(), currentNode->actions[expandIdx]->getIdx());
			return currentNode->children[expandIdx];
		}
		
		int selectedChildIdx = currentNode->selectAndGetIdx(exploreFactor);
		assert(selectedChildIdx < int(currentNode->children.size()));
		assert(selectedChildIdx < int(currentNode->actions.size()));

		actionHistory.emplace_back(currentNode->state->getTurn(), currentNode->actions[selectedChildIdx]->getIdx());
		currentNode = std::dynamic_pointer_cast<MCTSNode>(currentNode->children[selectedChildIdx]);
		assert(currentNode);
	}

	return currentNode;
}

int MCTSAgentWithMAST::expandAndGetIdx(const sp<MCTSNode>& node) {
	int expandedIdx = node->expandAndGetIdx();
	node->children[expandedIdx]->parent = node;
	return expandedIdx;
}

int MCTSAgentWithMAST::MCTSNode::expandAndGetIdx() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return nextActionToResolveIdx++;
}

sp<MCTSNodeBase> MCTSAgentWithMAST::MCTSNode::getChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)) {

}

int MCTSAgentWithMAST::MCTSNode::selectAndGetIdx(param_t exploreFactor) {
	assert(!children.empty());
	return std::max_element(children.begin(), children.end(),
			[&exploreFactor, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreFactor) < UCT(ch2, exploreFactor);
	}) - children.begin();
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

	// return state->getReward(getID());
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
	// we assume there are 2 agents, with rewards [0, 1]
	statToUpdate.score += agentRewards[id];
	++statToUpdate.times;
}

void MCTSAgentWithMAST::postWork() {
	for (auto& v : actionsStats)
		for (auto& x : v)
			x.score *= decayFactor;
}

std::vector<KeyValue> MCTSAgentWithMAST::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with UCT selection and MAST policy with epsilon-greedy simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
		{ "Epsilon constant (E) in MAST default policy", std::to_string(epsilon) },
		{ "Decay factor (gamma) in MAST global action table", std::to_string(decayFactor) }
	};
}
