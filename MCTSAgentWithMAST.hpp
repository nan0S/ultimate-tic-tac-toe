#ifndef MCTS_AGENT_WITH_MAST_HPP
#define MCTS_AGENT_WITH_MAST_HPP

#include "Agent.hpp"
#include "State.hpp"

class MCTSAgentWithMAST : public Agent {
public:
	using param_t = double;
	using reward_t = State::reward_t;

  	MCTSAgentWithMAST(AgentID id, const up<State> &initialState,
			double limitInMs, param_t exploreSpeed=1.0,
			param_t epsilon=0.3);

	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	std::vector<KeyValue> getDesc() const override;

	void changeTurnLimit(double newLimitInMs);

private:
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

	sp<MCTSNode> treePolicy();
	sp<MCTSNode> expand(const sp<MCTSNode>& node);
	reward_t defaultPolicy(const sp<MCTSNode>& initialNode);
	void backup(sp<MCTSNode> node, reward_t delta);

private:
	sp<MCTSNode> root;
	CalcTimer timer;
	double exploreSpeed;
	double epsilon;

	int descended;
	int simulationCount = 0;
};

#endif /* MCTS_AGENT_WITH_MAST_HPP */
