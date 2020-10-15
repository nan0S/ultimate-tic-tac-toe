#ifndef STATE_HPP
#define STATE_HPP

#include "Common.hpp"
#include "Action.hpp"
#include "Agent.hpp"

class State {
public:
	virtual bool isTerminal() const = 0;
	virtual void apply(const sp<Action>& action) = 0;
	up<State> applyCopy(const sp<Action>& action);

	virtual std::vector<sp<Action>> getValidActions() = 0;
	virtual bool isValid(const sp<Action>& action) const = 0;

	virtual up<State> clone() = 0;
	virtual bool didWin(AgentID id) const = 0;

	virtual std::ostream& print(std::ostream& out) const = 0;
	friend std::ostream& operator<<(std::ostream& out, const State& state);
	virtual std::string getWinnerName() const = 0;

	virtual ~State() = default;
};

#endif /* STATE_HPP */
