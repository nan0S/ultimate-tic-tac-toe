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

	// temporary
	assert(maxAgentCount == 2);
	assert(maxActionCount == 81);
	std::fill(actionsStats.begin(), actionsStats.end(), std::vector<MASTActionStats>(maxActionCount));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgentWithMAST::getAction(const up<State>&) {
	timer.startCalculation();

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		int delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
		++simulationCount;
	}

	timer.stopCalculation();
	return root->bestAction();	
}

sp<MCTSAgentWithMAST::MCTSNode> MCTSAgentWithMAST::treePolicy() {
	auto currentNode = root;
	descended = 0;
	while (!currentNode->isTerminal()) {
		++descended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = currentNode->selectChild(exploreSpeed);
	}
	return currentNode;
}

bool MCTSAgentWithMAST::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgentWithMAST::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSAgentWithMAST::MCTSNode> MCTSAgentWithMAST::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSAgentWithMAST::MCTSNode> MCTSAgentWithMAST::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));
	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<MCTSAgentWithMAST::MCTSNode> MCTSAgentWithMAST::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSAgentWithMAST::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

reward_t MCTSAgentWithMAST::defaultPolicy(const sp<MCTSNode>& initialNode) {
	auto state = initialNode->cloneState();

     while (!state->isTerminal()) {
		auto actions = state->getValidActions();
		const auto& action = Random::choice(actions);
		state->apply(action);
	}

	return state->getReward(getID());
	// return state->didWin(getID());
}

up<State> MCTSAgentWithMAST::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgentWithMAST::backup(sp<MCTSNode> node, reward_t delta) {
	int ascended = 0;
	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++ascended;
	}
	assert(descended + 1 == ascended);
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
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "Number of iterations", std::to_string(100) },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreSpeed) }
	};
}

void MCTSAgentWithMAST::changeCalcLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}
