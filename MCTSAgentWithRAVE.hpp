#ifndef MCTS_AGENT_WITH_RAVE_HPP
#define MCTS_AGENT_WITH_RAVE_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgentWithRAVE : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;
	using AgentArgs = Agent::AgentArgs;

	MCTSAgentWithRAVE(AgentID id, const up<State> &initialState,
		double calcLimitInMs, const AgentArgs& args);
 
	std::vector<KeyValue> getDesc() const override;

private:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> getChildFromState(up<State>&& state) override;
		int expandAndGetIdx();
		int selectAndGetIdx(param_t exploreFactor);

		struct ActionStats {
			reward_t score = 0;
			int times = 0;
		};

		std::vector<ActionStats> actionStats;
	};

	sp<MCTSNodeBase> treePolicy() override;
	int expandAndGetIdx(const sp<MCTSNode>& node);
	void defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	sp<Action> getActionWithDefaultPolicy(const up<State>& state);
	void backup(sp<MCTSNodeBase> node) override;

	void postWork() override;
	void RAVEPolicy();
	inline void updateActionStat(AgentID id, int actionIdx);

private:
	double KFactor;
	int maxActionCount;
	std::vector<int> treeActionHistory;
	std::vector<int> simActionHistory;
	int timesTreeDescended;
	int defaultPolicyLength;
};

#endif /* MCTS_AGENT_WITH_RAVE_HPP */
