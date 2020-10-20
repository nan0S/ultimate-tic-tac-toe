#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentBase::param_t;
using reward_t = MCTSAgentBase::reward_t;

MCTSAgentBase::MCTSAgentBase(AgentID id, up<MCTSAgentBase::MCTSNode>&& root, 
		double calcLimitInMs) :
	Agent(id),
	root(std::move(root)),
	timer(calcLimitInMs),
	maxAgentCount(this->root->state->getAgentCount()),
	agentRewards(maxAgentCount) {

}

MCTSAgentBase::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

MCTSAgentBase::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgentBase::getAction(const up<State>&) {
	timer.startCalculation();
	currentSimulationCount = 0;

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		defaultPolicy(selectedNode);
		backup(selectedNode);
		++simulationCount;
		++currentSimulationCount;
	}

	const auto result = root->getBestAction();
	postWork();
	timer.stopCalculation();

	return result;
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::treePolicy() {
	auto currentNode = root;
	timesTreeDescended = 0;

	while (!currentNode->isTerminal()) {
		++timesTreeDescended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = select(currentNode);
	}

	return currentNode;
}

bool MCTSAgentBase::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgentBase::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

int MCTSAgentBase::MCTSNode::expandGetIdx() {
	assert(shouldExpand());
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx];
	children.push_back(makeChildFromState(state->applyCopy(action)));
	return nextActionToResolveIdx++;
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::expand(const sp<MCTSNode>& node) {
	int expandIdx = expandGetIdx(node);
	assert(expandIdx == int(node->children.size()) - 1);
	return node->children[expandIdx];
}

int MCTSAgentBase::expandGetIdx(const sp<MCTSNode>& node) {
	int expandIdx = node->expandGetIdx();
	assert(expandIdx == int(node->children.size()) - 1);
	node->children[expandIdx]->parent = node;
	return expandIdx;
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::select(const sp<MCTSNode>& node) {
	int selectIdx = selectGetIdx(node);
	assert(selectIdx < int(node->children.size()));
	assert(selectIdx < int(node->actions.size()));
	return node->children[selectIdx];
}

int MCTSAgentBase::selectGetIdx(const sp<MCTSNode>& node) {
	const auto& children = node->children;
	const auto& actions = node->actions;
	assert(!children.empty());
	assert(children.size() <= actions.size());

	int selectIdx = 0;
	auto evaluation = eval(children[0], actions[0]);
	const int childCount = children.size();

	for (int i = 1; i < childCount; ++i) {
		auto curEvaluation = eval(children[i], actions[0]);
		if (curEvaluation > evaluation)
			evaluation = curEvaluation, selectIdx = i;
	}

	return selectIdx;
}

up<State> MCTSAgentBase::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgentBase::MCTSNode::addReward(reward_t agentPlayingReward, AgentID whoIsPlaying) {
	stats.score += whoIsPlaying != state->getTurn() ? agentPlayingReward : 1 - agentPlayingReward;
	++stats.visits;
}

sp<Action> MCTSAgentBase::MCTSNode::getBestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
		[](const auto& ch1, const auto& ch2){ return *ch1 < *ch2; }) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

bool MCTSAgentBase::MCTSNode::operator<(const MCTSNode& o) const {
	return stats.visits < o.stats.visits;
}

void MCTSAgentBase::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
		[&action](const auto& x){ return action->equals(x); }) - root->actions.begin();
	assert(recordActionIdx < int(root->actions.size()));

	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = root->makeChildFromState(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

void MCTSAgentBase::changeCalcLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}

void MCTSAgentBase::postWork() {

}
