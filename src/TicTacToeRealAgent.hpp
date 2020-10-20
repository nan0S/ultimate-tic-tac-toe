#ifndef TICTACTOE_REAL_AGENT_HPP
#define TICTACTOE_REAL_AGENT_HPP

#include "Common.hpp"
#include "Agent.hpp"

class TicTacToeRealAgent : public Agent {
public:
	TicTacToeRealAgent(AgentID id, double, const up<State>&, const AgentArgs&);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc(double avgSimulationCount=0) const override;
	
private:
	bool isInRange(int idx) const;
};

#endif /* TICTACTOE_REAL_AGENT_HPP */
