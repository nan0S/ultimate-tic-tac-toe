#include "RandomAgent.hpp"

#include <cassert>

RandomAgent::RandomAgent(AgentID id, const up<State>&, double, const AgentArgs&) : Agent(id) {

}

sp<Action> RandomAgent::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}

std::vector<KeyValue> RandomAgent::getDesc() const {
	return { {"Random agent with uniform distribution.", ""} };
}
