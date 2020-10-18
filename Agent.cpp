#include "Agent.hpp"

#include <cassert>

namespace std {
	std::string to_string(AgentID id) {
		switch (id) {
			case NONE:
				return "NONE";
			case AGENT1:
				return "AGENT1";
			case AGENT2:
				return "AGENT2";
		}
		assert(false);
	}
}

Agent::Agent(AgentID id) : id(id) {

}

AgentID Agent::getID() const {
	return id;
}

std::vector<KeyValue> Agent::getDesc() const {
	return {};
}

void Agent::recordAction(const sp<Action>&) {

}
