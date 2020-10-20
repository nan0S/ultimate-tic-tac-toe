#ifndef MCTS_AGENT_WITH_RAVE_HPP
#define MCTS_AGENT_WITH_RAVE_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgentWithRAVE : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

	MCTSAgentWithRAVE(AgentID id, double calcLimitInMs,
		const up<State> &initialState, const AgentArgs& args);
 
	std::vector<KeyValue> getDesc(double avgSimulationCount=0) const override;

protected:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> makeChildFromState(up<State>&& state) override;

		struct RAVEActionStats {
			reward_t reward = 0;
			int visits = 0;
		};

		std::vector<RAVEActionStats> actionsStats;
	};

	sp<MCTSNodeBase> expand(const sp<MCTSNodeBase>& node) override;
	sp<MCTSNodeBase> select(const sp<MCTSNodeBase>& node) override;
	param_t eval(const sp<MCTSNodeBase>& node, const sp<Action>& action) override;

	void defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	void backup(sp<MCTSNodeBase> node) override;

private:
	param_t exploreFactor;
	param_t KFactor;

	int maxActionCount;
	std::vector<int> actionHistory;
	int defaultPolicyLength;
};

#endif /* MCTS_AGENT_WITH_RAVE_HPP */
