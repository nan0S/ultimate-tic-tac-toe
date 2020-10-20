#ifndef MCTS_AGENT_WITH_MAST_HPP
#define MCTS_AGENT_WITH_MAST_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgentWithMAST : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

	MCTSAgentWithMAST(AgentID id, double calcLimitInMs,
		const up<State> &initialState, const AgentArgs& args);
 
	std::vector<KeyValue> getDesc(double avgSimulationCount=0) const override;

protected:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> makeChildFromState(up<State>&& state) override;
	};

	struct MASTActionStats {
		reward_t score = 0;
		int times = 0;
	};

	sp<MCTSNodeBase> expand(const sp<MCTSNodeBase>& node) override;
	sp<MCTSNodeBase> select(const sp<MCTSNodeBase>& node) override;
	param_t eval(const sp<MCTSNodeBase>& node, const sp<Action>& action) override;

	void defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	sp<Action> getActionWithDefaultPolicy(const up<State>& state);
	void backup(sp<MCTSNodeBase> node) override;
	void MASTPolicy();
	inline void updateActionStat(AgentID id, int actionIdx);

	void postWork() override;

private:
	param_t exploreFactor;
	param_t epsilon;
	param_t decayFactor;

	int maxActionCount;
	std::vector<std::vector<MASTActionStats>> actionsStats;
	std::vector<std::pair<AgentID, int>> actionHistory;
	int defaultPolicyLength;
};

#endif /* MCTS_AGENT_WITH_MAST_HPP */
