#include "State.hpp"

std::ostream& operator<<(std::ostream& out, const State& state) {
	return state.print(out);
}
