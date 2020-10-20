#ifndef MCTS_AGENT_HPP
#define MCTS_AGENT_HPP

#include "MCTSAgentBase.hpp"
#include "State.hpp"

class MCTSAgent : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

  	MCTSAgent(AgentID id, const up<State> &initialState,
			double calcLimitInMs, const AgentArgs& args);

	std::vector<KeyValue> getDesc() const override;

protected:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);
		sp<MCTSNodeBase> makeChildFromState(up<State>&& state) override;
	};

	param_t eval(const sp<MCTSNodeBase>& node, const sp<Action>& action) override;
	void defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	void backup(sp<MCTSNodeBase> node) override;

private:
	param_t exploreFactor;
};

#endif /* MCTS_AGENT_HPP */
