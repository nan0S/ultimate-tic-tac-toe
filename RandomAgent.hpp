#ifndef RANDOM_AGENT_HPP
#define RANDOM_AGENT_HPP

#include "Agent.hpp"
#include "State.hpp"

class RandomAgent : public Agent {
public:
	RandomAgent(AgentID id);
	sp<Action> getAction(const up<State>& state) override;
	std::map<std::string, std::string> getDesc() const override;
};

#endif /* RANDOM_AGENT_HPP */
