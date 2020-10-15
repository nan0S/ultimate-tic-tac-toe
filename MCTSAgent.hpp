#ifndef MCTS_AGENT_HPP
#define MCTS_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"

class MCTSAgent : public Agent {
public:
	using param_t = double;
	using reward_t = int;

  	MCTSAgent(AgentID id, const up<State> &initialState,
			param_t exploreSpeed = 1.0, int numberOfIters = 1000);
	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	std::map<std::string, std::string> getDesc() const override;

private:
	int numberOfIters;
	double exploreSpeed;

	struct MCTSNode {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		inline bool isTerminal() const;
		inline bool shouldExpand() const;
		sp<MCTSNode> expand();
		sp<MCTSNode> selectChild(param_t exploreSpeed=1.0);
		param_t UCT(const sp<MCTSNode>& v, param_t c=1.0) const;
		up<State> cloneState();
		void addReward(reward_t delta);
		sp<Action> bestAction();
		bool operator<(const MCTSNode& o) const;

		up<State> state;
		wp<MCTSNode> parent;
		std::vector<sp<MCTSNode>> children;
		std::vector<sp<Action>> actions;
		int nextActionToResolveIdx = 0;
		
		struct MCTSNodeStats {
			reward_t score = 0;
			int visits = 0;
		} stats;
	};

	int descended;
	sp<MCTSNode> root;

	sp<MCTSNode> treePolicy();
	sp<MCTSNode> expand(const sp<MCTSNode>& node);
	int defaultPolicy(const sp<MCTSNode>& initialNode);
	void backup(sp<MCTSNode> node, reward_t delta);
};

#endif /* MCTS_AGENT_HPP */

/*
 * node reprezentuje stan
 * node zawiera statystyki: win, total
 * node zawiera liste expanded dzieci
 * node zawiera liste zszufflowanych akcji poprawnych w stanie, ktory reprezentuje
 * node zawiera stan
 * node zawiera iterator/index do nastepnej akcji ktora uzyjemy w expand
*/
