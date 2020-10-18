#include "MCTSAgentWithMAST.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithMAST::param_t;
using reward_t = MCTSAgentWithMAST::reward_t;

MCTSAgentWithMAST::MCTSAgentWithMAST(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, param_t exploreSpeed, param_t epsilon) :
	Agent(id),
	root(std::mksh<MCTSNode>(initialState)),
	timer(calcLimitInMs),
	exploreSpeed(exploreSpeed),
	epsilon(epsilon),
     maxAgentCount(initialState->getAgentCount()),
	maxActionCount(initialState->getActionCount()),
	actionsStats(maxAgentCount) {

	std::fill(actionsStats.begin(), actionsStats.end(),
			std::vector<MASTActionStats>(maxActionCount));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgentWithMAST::getAction(const up<State>&) {
	timer.startCalculation();

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		reward_t delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
		++simulationCount;
	}

	timer.stopCalculation();
	return root->bestAction();	
}

sp<MCTSAgentWithMAST::MCTSNode> MCTSAgentWithMAST::treePolicy() {
	auto currentNode = root;
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
		
		int selectedChildIdx = currentNode->selectAndGetIdx(exploreSpeed);
		assert(selectedChildIdx < int(currentNode->children.size()));
		assert(selectedChildIdx < int(currentNode->actions.size()));
		actionHistory.emplace_back(currentNode->state->getTurn(), currentNode->actions[selectedChildIdx]->getIdx());
		currentNode = currentNode->children[selectedChildIdx];
	}
	return currentNode;
}

bool MCTSAgentWithMAST::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgentWithMAST::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
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

MCTSAgentWithMAST::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

int MCTSAgentWithMAST::MCTSNode::selectAndGetIdx(param_t exploreSpeed) {
	assert(!children.empty());
	return std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	}) - children.begin();
}

param_t MCTSAgentWithMAST::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

reward_t MCTSAgentWithMAST::defaultPolicy(const sp<MCTSNode>& initialNode) {
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

up<State> MCTSAgentWithMAST::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgentWithMAST::backup(sp<MCTSNode> node, reward_t delta) {
	int timesTreeAscended = 0;

	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	MASTPolicy(delta);
}

void MCTSAgentWithMAST::MASTPolicy(reward_t delta) {
	assert(defaultPolicyLength + timesTreeDescended == int(actionHistory.size()));
	assert((delta == 1 && actionHistory.back().first == getID()) || 
		  (delta == 0 && actionHistory.back().first != getID()) ||
		   delta == 0.5);

	for (const auto& [agentID, actionIdx] : actionHistory)
		updateActionStat(agentID, actionIdx, delta);

	actionHistory.clear();
}

void MCTSAgentWithMAST::updateActionStat(AgentID id, int actionIdx, reward_t delta) {
	assert(id < int(actionsStats.size()));
	assert(actionIdx < int(actionsStats[id].size()));

	auto& statToUpdate = actionsStats[id][actionIdx];
	statToUpdate.score += delta;
	++statToUpdate.times;
}

void MCTSAgentWithMAST::MCTSNode::addReward(reward_t delta) {
	stats.score += delta;
	++stats.visits;
}

sp<Action> MCTSAgentWithMAST::MCTSNode::bestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

bool MCTSAgentWithMAST::MCTSNode::operator<(const MCTSNode& o) const {
	// return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
	return stats.visits < o.stats.visits;
}

void MCTSAgentWithMAST::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
			[&action](const auto& x){
		return action->equals(x);
	}) - root->actions.begin();

	assert(recordActionIdx < int(root->actions.size()));
	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = std::mksh<MCTSNode>(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

std::vector<KeyValue> MCTSAgentWithMAST::getDesc() const {
	auto averageSimulationCount = double(simulationCount) / timer.getTotalNumberOfCals();
	return { { "MCTS Agent with UCT selection and MAST policy with epsilon-greedy simulation.", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreSpeed) },
		{ "Epsilon constant (E) in MAST default policy", std::to_string(epsilon) }
	};
}

void MCTSAgentWithMAST::changeCalcLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}
