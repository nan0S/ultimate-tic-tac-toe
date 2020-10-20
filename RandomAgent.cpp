#include "RandomAgent.hpp"

#include <cassert>

RandomAgent::RandomAgent(AgentID id, double calcLimitInMs, const up<State>&, const AgentArgs&) :
	Agent(id, calcLimitInMs) {

}

sp<Action> RandomAgent::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}

std::vector<KeyValue> RandomAgent::getDesc(double) const {
	return { {"Random agent with uniform distribution.", ""} };
}
