#include "MCTSPlayer.hpp"

#include <cassert>
#include <algorithm>

using param_t = MCTSPlayer::param_t;
using reward_t = MCTSPlayer::reward_t;

MCTSPlayer::MCTSPlayer(const up<State>& initialState, param_t exploreSpeed, int numberOfIters)
	: numberOfIters(numberOfIters), exploreSpeed(exploreSpeed), root(std::mksh<MCTSNode>(initialState)) {

}

MCTSPlayer::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {

}

sp<Action> MCTSPlayer::getAction(const up<State>& state) {
	for (int i = 0; i < numberOfIters; ++i) {
		auto selectedNode = treePolicy();
		int delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
	}
	return root->bestAction();	
}

sp<MCTSPlayer::MCTSNode> MCTSPlayer::treePolicy() {
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

bool MCTSPlayer::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSPlayer::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSPlayer::MCTSNode> MCTSPlayer::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSPlayer::MCTSNode> MCTSPlayer::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));
	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

MCTSPlayer::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {

}

sp<MCTSPlayer::MCTSNode> MCTSPlayer::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSPlayer::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

int MCTSPlayer::defaultPolicy(const sp<MCTSNode>& initialNode) {
	auto state = initialNode->cloneState();
	state->record();
	auto actions = state->getValidActions();
     int actionIdx = 0;
     while (!state->isTerminal()) {
		// we assume that in initialState we get all valid actions which will become invalid
		// after some actions are done, but no other will come
		// it's not general but in UltimateTicTacToe it's true
		while (!state->isValid(actions[actionIdx])) {
			++actionIdx;
			assert(actionIdx < int(actions.size()));
		}
		const auto& action = actions[actionIdx];
		state->apply(action);
	}
	return state->didWon();
}

up<State> MCTSPlayer::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSPlayer::backup(sp<MCTSNode> node, reward_t delta) {
	int ascended = 0;
	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++ascended;
	}
	assert(descended + 1 == ascended);
}

void MCTSPlayer::MCTSNode::addReward(reward_t delta) {
	stats.score += delta;
	++stats.visits;
}

sp<Action> MCTSPlayer::MCTSNode::bestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

// bool MCTSPlayer::MCTSNode::operator<(const MCTSNode& o) const {
	// return stats.visits < o.stats.visits;
// }
bool MCTSPlayer::MCTSNode::operator<(const MCTSNode& o) const {
	return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
}
// #include "UltimateTicTacToe.hpp"

void MCTSPlayer::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
			[&action](const auto& x){
		return action->equals(x);
	}) - root->actions.begin();

	// const auto& yy = std::dynamic_pointer_cast<UltimateTicTacToe::UltimateTicTacToeAction>(action);
	// std::cout << yy->row << " " << yy->col << " " << yy->action.row << " " << yy->action.col << std::endl;
	// std::cout << root->actions.size() << std::endl;
	// for (const auto& x : root->actions) {
		// const auto& y = std::dynamic_pointer_cast<UltimateTicTacToe::UltimateTicTacToeAction>(x);
		// std::cout << y->row << " " << y->col << " " << y->action.row << " " << y->action.col << std::endl;
	// }
	// exit(0);
	assert(recordActionIdx < int(root->actions.size()));
	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = std::mksh<MCTSNode>(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

std::map<std::string, std::string> MCTSPlayer::getDesc() const {
	return { { "MCTS Player with UCT selection and random simulation policy.", "" },
		{ "Number of iterations", std::to_string(numberOfIters) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreSpeed) }
	};
}
