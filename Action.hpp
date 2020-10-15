#ifndef ACTION_HPP
#define ACTION_HPP

#include "Common.hpp"

struct Action {
	virtual bool equals(const sp<Action>& o) const = 0;
	virtual ~Action() = default;
};

#endif /* ACTION_HPP */
