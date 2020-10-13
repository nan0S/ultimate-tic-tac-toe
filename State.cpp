#include "State.hpp"

std::ostream& operator<<(std::ostream& out, const State& state) {
	return state.print(out);
}

up<State> State::applyCopy(const sp<Action>& action) {
	auto ptr = clone();
	ptr->apply(action);
	return ptr;
}
