#ifndef AGENT_HPP
#define AGENT_HPP

#include "Common.hpp"
#include "Action.hpp"

#include <vector>
#include <map>

enum AgentID {
	NONE = -1, AGENT1, AGENT2
};

namespace std {
	std::string to_string(AgentID id);
}

class State;

class Agent {
public:
	using param_t = double;
	using AgentArgs = std::map<std::string, param_t>;

	Agent(AgentID id, double calcLimitInMs);
	AgentID getID() const;

	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::vector<KeyValue> getDesc(double avgSimulationCount=0) const;
	virtual double getAvgSimulationCount() const;
	void changeCalcLimit(double newCalcLimit);
	virtual param_t getOrDefault(const AgentArgs& args, const std::string& key, 
		param_t defaultVal) const;

	virtual ~Agent() = default;

protected:
	AgentID id;
	CalcTimer timer;
	int simulationCount = 0;
};

#endif /* AGENT_HPP */
