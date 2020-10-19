#include "MCTSAgentWithRAVE.hpp"
#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithRAVE::param_t;
using reward_t = MCTSAgentWithRAVE::reward_t;
using MCTSNodeBase = MCTSAgentWithRAVE::MCTSNodeBase;


MCTSAgentWithRAVE::MCTSAgentWithRAVE(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args),
	KFactor(getOrDefault(args, "KFactor", 3)),
	maxActionCount(initialState->getActionCount()) {

}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgentWithRAVE::treePolicy() {
	auto currentNode = std::dynamic_pointer_cast<MCTSNode>(root);
	assert(currentNode);
	timesTreeDescended = 0;

	while (!currentNode->isTerminal()) {
		++timesTreeDescended;

		if (currentNode->shouldExpand()) {
			int expandIdx = expandAndGetIdx(currentNode);
			assert(expandIdx == int(currentNode->children.size() - 1));
			assert(expandIdx == currentNode->nextActionToResolveIdx - 1);

			treeActionHistory.emplace_back(
				currentNode->state->getTurn(),
				currentNode->actions[expandIdx]->getIdx());

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

int MCTSAgentWithRAVE::expandAndGetIdx(const sp<MCTSNode>& node) {
	int expandedIdx = node->expandAndGetIdx();
	node->children[expandedIdx]->parent = node;
	return expandedIdx;
}

int MCTSAgentWithRAVE::MCTSNode::expandAndGetIdx() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return nextActionToResolveIdx++;
}

sp<MCTSNodeBase> MCTSAgentWithRAVE::MCTSNode::getChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithRAVE::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)) {

}

int MCTSAgentWithRAVE::MCTSNode::selectAndGetIdx(param_t exploreFactor) {
	assert(!children.empty());
	return std::max_element(children.begin(), children.end(),
			[&exploreFactor, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreFactor) < UCT(ch2, exploreFactor);
	}) - children.begin();
}

reward_t MCTSAgentWithRAVE::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();
	defaultPolicyLength = 0;

	while (!state->isTerminal()) {
		const auto action = getActionWithDefaultPolicy(state);
		actionHistory.emplace_back(state->getTurn(), action->getIdx());
		state->apply(action);
		++defaultPolicyLength;
	}

	return state->getReward(getID());
}

sp<Action> MCTSAgentWithRAVE::getActionWithDefaultPolicy(const up<State>& state) {
	auto actions = state->getValidActions();
	// std::shuffle()
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

void MCTSAgentWithRAVE::backup(sp<MCTSNodeBase> node, reward_t delta) {
	int timesTreeAscended = 0;

	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	RAVEPolicy(delta);
}

void MCTSAgentWithRAVE::RAVEPolicy(reward_t delta) {
	assert(defaultPolicyLength + timesTreeDescended == int(actionHistory.size()));
	// temporary
	assert((delta == 1 && actionHistory.back().first == getID()) || 
		  (delta == 0 && actionHistory.back().first != getID()) ||
		   delta == 0.5);

	for (const auto& [agentID, actionIdx] : actionHistory)
		updateActionStat(agentID, actionIdx, delta);

	actionHistory.clear();
}

void MCTSAgentWithRAVE::updateActionStat(AgentID id, int actionIdx, reward_t delta) {
	assert(id < int(actionsStats.size()));
	assert(actionIdx < int(actionsStats[id].size()));

	auto& statToUpdate = actionsStats[id][actionIdx];
	statToUpdate.score += delta;
	++statToUpdate.times;
}

void MCTSAgentWithRAVE::postWork() {
	for (auto& v : actionsStats)
		for (auto& x : v)
			x.score *= decayFactor;
}

std::vector<KeyValue> MCTSAgentWithRAVE::getDesc() const {
	int averageSimulationCount = std::round(double(simulationCount) / timer.getTotalNumberOfCals());
	int averageSpeedSimPerSec = std::round((simulationCount * 1000.0) / timer.getTotalCalcTime());
	return { { "MCTS Agent with UCT selection and RAVE policy with epsilon-greedy simulation.", "" },
		{ "", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) + " sim/turn" },
		{ "Average simulation/s speed", std::to_string(averageSpeedSimPerSec) + " sim/sec" },
		{ "", "" },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
		{ "Epsilon constant (E) in RAVE default policy", std::to_string(epsilon) },
		{ "Decay factor (gamma) in RAVE global action table", std::to_string(decayFactor) }
	};
}
