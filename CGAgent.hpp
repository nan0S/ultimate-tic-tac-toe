#ifndef CGAGENT_HPP
#define CGAGENT_HPP

#include "Agent.hpp"
#include "State.hpp"

class CGAgent : public Agent {
public:
	CGAgent(AgentID id);
	sp<Action> getAction(const up<State>& state) override;
};

#endif /* CGAGENT_HPP */
