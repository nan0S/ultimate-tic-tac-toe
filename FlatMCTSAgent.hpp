#ifndef FLAT_MCTS_AGENT_HPP
#define FLAT_MCTS_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"
#include "Common.hpp"
#include "Action.hpp"

#include <vector>

class FlatMCTSAgent : public Agent {
public:
	FlatMCTSAgent(AgentID id, const int numberOfIters=100);
	sp<Action> getAction(const up<State>& state) override;
	std::map<std::string, std::string> getDesc() const override;
	
	struct ActionStats {
		int winCount = 0;
		int total = 0;
		bool operator<(const ActionStats& o) const;
	};

private:
	const int numberOfIters;
	std::vector<ActionStats> stats;
};

#endif /* MCTS_AGENT_HPP */
