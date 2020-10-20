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

using param_t = Agent::param_t;

Agent::Agent(AgentID id, double calcLimitInMs) : id(id), timer(calcLimitInMs) {

}

AgentID Agent::getID() const {
	return id;
}

std::vector<KeyValue> Agent::getDesc(double) const {
	return {};
}

void Agent::recordAction(const sp<Action>&) {

}

param_t Agent::getOrDefault(const AgentArgs& args, const std::string& key,
	param_t defaultVal) const {
	return args.count(key) ? args.at(key) : defaultVal;
}

double Agent::getAvgSimulationCount() const {
	if (timer.getTotalNumberOfCals() == 0)
		return 0;
	return double(simulationCount) / timer.getTotalNumberOfCals();
}

void Agent::changeCalcLimit(double newCalcLimit) {
	timer.changeLimit(newCalcLimit);
}
