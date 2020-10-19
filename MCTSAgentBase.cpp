#include "MCTSAgentBase.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgentBase::param_t;
using reward_t = MCTSAgentBase::reward_t;

MCTSAgentBase::MCTSAgentBase(AgentID id, up<MCTSAgentBase::MCTSNode>&& root, 
		double calcLimitInMs, const AgentArgs& args) :
	Agent(id),
	root(std::move(root)),
	timer(calcLimitInMs),
	maxAgentCount(this->root->state->getAgentCount()),
	agentRewards(maxAgentCount),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)) {

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

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		defaultPolicy(selectedNode);
		backup(selectedNode);
		++simulationCount;
	}

	const auto result = root->getBestAction();
	postWork();
	timer.stopCalculation();

	return result;
}

bool MCTSAgentBase::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgentBase::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(getChildFromState(state->applyCopy(action)));
	// children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSAgentBase::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
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
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

bool MCTSAgentBase::MCTSNode::operator<(const MCTSNode& o) const {
	// return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
	return stats.visits < o.stats.visits;
}

void MCTSAgentBase::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
			[&action](const auto& x){
		return action->equals(x);
	}) - root->actions.begin();

	assert(recordActionIdx < int(root->actions.size()));
	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = root->getChildFromState(root->state->applyCopy(action));
		// root = std::mksh<MCTSNode>(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

void MCTSAgentBase::changeCalcLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}

void MCTSAgentBase::postWork() {

}
