#ifndef AGENT_HPP
#define AGENT_HPP

#include "Common.hpp"
#include "Action.hpp"

#include <vector>

enum AgentID {
	NONE = -1, AGENT1, AGENT2
};

namespace std {
	std::string to_string(AgentID id);
}

class State;

class Agent {
public:
	Agent(AgentID id);
	AgentID getID() const;
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::vector<KeyValue> getDesc() const;
	virtual ~Agent() = default;

private:
	AgentID id;
};

#endif /* AGENT_HPP */
