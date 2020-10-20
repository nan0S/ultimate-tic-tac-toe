#ifndef FLAT_MCTS_AGENT_HPP
#define FLAT_MCTS_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"
#include "Common.hpp"
#include "Action.hpp"

#include <vector>

class FlatMCTSAgent : public Agent {
public:
	using reward_t = State::reward_t;

	FlatMCTSAgent(AgentID id, double calcLimitInMs, const up<State>&, const AgentArgs&);

	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc(double avgSimulationCount=0) const override;
	
	struct ActionStats {
		reward_t reward = 0;
		int total = 0;
		bool operator<(const ActionStats& o) const;
	};

private:
	std::vector<ActionStats> stats;
};

#endif /* MCTS_AGENT_HPP */
