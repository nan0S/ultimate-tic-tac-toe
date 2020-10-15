#ifndef AGENT_HPP
#define AGENT_HPP

#include "Common.hpp"
// #include "State.hpp"
#include "Action.hpp"

#include <map>

enum AgentID {
	NONE = -1, AGENT1, AGENT2
};

class State;

class Agent {
public:
	Agent(AgentID id);
	AgentID getID() const;
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::map<std::string, std::string> getDesc() const;
	virtual ~Agent() = default;

private:
	AgentID id;
};

#endif /* AGENT_HPP */
