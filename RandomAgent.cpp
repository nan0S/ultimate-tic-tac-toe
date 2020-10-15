#include "RandomAgent.hpp"

#include <cassert>

RandomAgent::RandomAgent(AgentID id) : Agent(id) {

}

sp<Action> RandomAgent::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}

std::map<std::string, std::string> RandomAgent::getDesc() const {
	return { {"Random agent with uniform distribution.", ""} };
}
