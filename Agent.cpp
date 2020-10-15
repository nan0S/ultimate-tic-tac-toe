#include "Agent.hpp"

Agent::Agent(AgentID id) : id(id) {

}

AgentID Agent::getID() const {
	return id;
}

std::map<std::string, std::string> Agent::getDesc() const {
	return {};
}

void Agent::recordAction(const sp<Action>&) {

}
