#ifndef MCTS_AGENT_BASE_HPP
#define MCTS_AGENT_BASE_HPP

#include "Agent.hpp"
#include "State.hpp"

class MCTSAgentBase : public Agent {
public:
	using param_t = Agent::param_t;
	using reward_t = State::reward_t;

protected:
	struct MCTSNode {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		bool isTerminal() const;
		bool shouldExpand() const;
		int expandGetIdx();

		virtual sp<MCTSNode> makeChildFromState(up<State>&& state) = 0;
		void addReward(reward_t agentPlayingReward, AgentID whoIsPlaying);
		sp<Action> getBestAction();
		up<State> cloneState();
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

public:
	MCTSAgentBase(AgentID id, up<MCTSNode>&& root,
		double calcLimitInMs);

	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	void changeCalcLimit(double newLimitInMs);
	double getAvgSimulationCount() const override;

protected:
	virtual sp<MCTSNode> treePolicy();
	virtual sp<MCTSNode> expand(const sp<MCTSNode>& node);
	int expandGetIdx(const sp<MCTSNode>& node);
	virtual sp<MCTSNode> select(const sp<MCTSNode>& node);
	int selectGetIdx(const sp<MCTSNode>& node);
	virtual param_t eval(const sp<MCTSNode>& node, const sp<Action>& action) = 0;
	virtual void defaultPolicy(const sp<MCTSNode>& initialNode) = 0;
	virtual void backup(sp<MCTSNode> node) = 0;
	virtual void postWork();

protected:
	sp<MCTSNode> root;
	CalcTimer timer;
	int maxAgentCount;
	std::vector<reward_t> agentRewards;

	int timesTreeDescended;
	int simulationCount = 0;
	int currentSimulationCount;
};

#endif /* MCTS_AGENT_BASE_HPP */
