#ifndef MCTS_AGENT_WITH_MAST_HPP
#define MCTS_AGENT_WITH_MAST_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgentWithMAST : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

	MCTSAgentWithMAST(AgentID id, const up<State> &initialState,
			double calcLimitInMs, param_t exploreFactor=1.0,
			param_t epsilon=0.8, param_t decayFactor=0.2);

	std::vector<KeyValue> getDesc() const override;

private:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> getChildFromState(up<State>&& state) override;
		int expandAndGetIdx();
		int selectAndGetIdx(param_t exploreFactor);
	};

	struct MASTActionStats {
		reward_t score = 0;
		int times = 0;
	};

	sp<MCTSNodeBase> treePolicy() override;
	int expandAndGetIdx(const sp<MCTSNode>& node);
	reward_t defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	sp<Action> getActionWithDefaultPolicy(const up<State>& state);
	void backup(sp<MCTSNodeBase> node, reward_t delta) override;

	void postWork() override;
	void MASTPolicy(reward_t delta);
	inline void updateActionStat(AgentID id, int actionIdx, reward_t delta);

private:
	double epsilon;
	double decayFactor;
	int timesTreeDescended;
	int maxAgentCount;
	int maxActionCount;
	std::vector<std::vector<MASTActionStats>> actionsStats;
	std::vector<std::pair<AgentID, int>> actionHistory;
	int defaultPolicyLength;
};

#endif /* MCTS_AGENT_WITH_MAST_HPP */
