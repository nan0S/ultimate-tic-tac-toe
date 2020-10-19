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
		sp<MCTSNode> expand();
		sp<MCTSNode> selectChild(param_t exploreFactor=1.0);

		virtual sp<MCTSNode> getChildFromState(up<State>&& state) = 0;

		param_t UCT(const sp<MCTSNode>& v, param_t c=1.0) const;
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
		double calcLimitInMs, const AgentArgs& args);

	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	void changeCalcLimit(double newLimitInMs);

protected:
	virtual sp<MCTSNode> treePolicy() = 0;
	sp<MCTSNode> expand(const sp<MCTSNode>& node);
	virtual void defaultPolicy(const sp<MCTSNode>& initialNode) = 0;
	virtual void backup(sp<MCTSNode> node) = 0;
	virtual void postWork();

protected:
	sp<MCTSNode> root;
	CalcTimer timer;
	int maxAgentCount;
	std::vector<reward_t> agentRewards;
	double exploreFactor;

	int descended;
	int simulationCount = 0;
};

#endif /* MCTS_AGENT_BASE_HPP */
