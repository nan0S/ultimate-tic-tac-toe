#ifndef MCTS_AGENT_WITH_RAVE_HPP
#define MCTS_AGENT_WITH_RAVE_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgentWithRAVE : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

	MCTSAgentWithRAVE(AgentID id, const up<State> &initialState,
			double calcLimitInMs, const AgentArgs& args);
 
	std::vector<KeyValue> getDesc() const override;

protected:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> makeChildFromState(up<State>&& state) override;

		struct RAVENodeStats {
			reward_t reward;
		};

		std::vector<
	};

	sp<MCTSNodeBase> expand(const sp<MCTSNodeBase>& node) override;
	sp<MCTSNodeBase> select(const sp<MCTSNodeBase>& node) override;
	param_t eval(const sp<MCTSNodeBase>& node) override;

	void defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	void backup(sp<MCTSNodeBase> node) override;
	void RAVEPolicy();
	void postWork() override;

private:
	param_t KFactor;

	int maxActionCount;
	std::vector<int> treeActionHistoryIdx;
	std::vector<int> simActionHistoryIdx;
	int defaultPolicyLength;
};

#endif /* MCTS_AGENT_WITH_RAVE_HPP */
