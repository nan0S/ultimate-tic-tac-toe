#ifndef RANDOM_AGENT_HPP
#define RANDOM_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"

class RandomAgent : public Agent {
public:
	RandomAgent(AgentID id, double calcLimitInMs, const up<State>&, const AgentArgs&);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc(double avgSimulationCount=0) const override;
};

#endif /* RANDOM_AGENT_HPP */
