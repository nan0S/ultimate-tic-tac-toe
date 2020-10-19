#ifndef RANDOM_AGENT_HPP
#define RANDOM_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"

class RandomAgent : public Agent {
public:
	RandomAgent(AgentID id, const up<State>&, double, const AgentArgs&);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
};

#endif /* RANDOM_AGENT_HPP */
