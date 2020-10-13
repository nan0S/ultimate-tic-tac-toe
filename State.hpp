#ifndef STATE_HPP
#define STATE_HPP

#include "Common.hpp"
#include "Action.hpp"

class State {
public:
	virtual bool isTerminal() const = 0;
	virtual void apply(const sp<Action>& action) = 0;
	virtual std::vector<sp<Action>> getValidActions() = 0;

	virtual std::ostream& print(std::ostream& out) const = 0;
	friend std::ostream& operator<<(std::ostream& out, const State& state);

	virtual std::string getWinnerName() const = 0;
};

#endif /* STATE_HPP */
