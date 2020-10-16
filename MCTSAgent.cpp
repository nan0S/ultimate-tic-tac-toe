#include "MCTSAgent.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;

MCTSAgent::MCTSAgent(AgentID id, const up<State>& initialState, 
		int numberOfIters, param_t exploreSpeed) :
	Agent(id), numberOfIters(numberOfIters),
	exploreSpeed(exploreSpeed), root(std::mksh<MCTSNode>(initialState)) {

}

MCTSAgent::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgent::getAction(const up<State>&) {
	for (int i = 0; i < numberOfIters; ++i) {
		auto selectedNode = treePolicy();
		int delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
	}
	return root->bestAction();	
}

sp<MCTSAgent::MCTSNode> MCTSAgent::treePolicy() {
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

bool MCTSAgent::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgent::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSAgent::MCTSNode> MCTSAgent::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSAgent::MCTSNode> MCTSAgent::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));
	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

MCTSAgent::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<MCTSAgent::MCTSNode> MCTSAgent::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSAgent::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

int MCTSAgent::defaultPolicy(const sp<MCTSNode>& initialNode) {
	auto state = initialNode->cloneState();
	// auto actions = state->getValidActions();
	// std::shuffle(actions.begin(), actions.end(), Random::rng);

     // int actionIdx = 0;
     while (!state->isTerminal()) {
		// we assume that in initialState we get all valid actions which will become invalid
		// after some actions are done, but no other will come
		// it's not general but in UltimateTicTacToe it's true
		// while (!state->isValid(actions[actionIdx])) {
			// ++actionIdx;
			// assert(actionIdx < int(actions.size()));
		// }
		auto actions = state->getValidActions();
		// const auto& action = actions[actionIdx];
		const auto& action = Random::choice(actions);
		state->apply(action);
	}
	return state->didWin(getID());
}

up<State> MCTSAgent::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgent::backup(sp<MCTSNode> node, reward_t delta) {
	int ascended = 0;
	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++ascended;
	}
	assert(descended + 1 == ascended);
}

void MCTSAgent::MCTSNode::addReward(reward_t delta) {
	stats.score += delta;
	++stats.visits;
}

sp<Action> MCTSAgent::MCTSNode::bestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

// bool MCTSAgent::MCTSNode::operator<(const MCTSNode& o) const {
	// return stats.visits < o.stats.visits;
// }
bool MCTSAgent::MCTSNode::operator<(const MCTSNode& o) const {
	// return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
	return stats.visits < o.stats.visits;
}
// #include "UltimateTicTacToe.hpp"

void MCTSAgent::recordAction(const sp<Action>& action) {
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

std::map<std::string, std::string> MCTSAgent::getDesc() const {
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "Number of iterations", std::to_string(numberOfIters) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreSpeed) }
	};
}
