#include "MCTSPlayer.hpp"

MCTSPlayer::MCTSPlayer(const up<State>& initialState, int numberOfIters, double C)
	: root(std::mku<MCTSNode>(initialState)), numberOfIters(numberOfIters), C(C) {

}

sp<Action> MCTSPlayer::getAction(const up<State>& state) {
	for (int i = 0; i < numberOfIters; ++i) {
		auto selectedNode = treePolicy();
		
	}
}

void MCTSPlayer::recordAction(const sp<Action>& action) {
	
}

sp<MCTSPlayer::MCTSNode> MCTSPlayer::treePolicy() {
	sp<MCTSNode> currentNode = root;
	while (!currentNode->isTerminal()) {
		if (currentNode->shouldExpand())
			return currentNode->expand();
		currentNode = currentNode->bestChild();
	}

}

std::map<std::string, std::string> MCTSPlayer::getDesc() const {
	return { { "MCTS Player with UCT selection and random simulation policy.", "" },
		{ "Number of iterations", std::to_string(numberOfIters) },
		{ "C constant in UCT policy", std::to_string(C) }
	};
}

MCTSPlayer::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
}

MCTSPlayer::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
}

bool MCTSPlayer::MCTSNode::isTerminal() const {
	return state->isTerminal();
}
