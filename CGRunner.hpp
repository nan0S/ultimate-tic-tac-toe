#ifndef CGRUNNER_HPP
#define CGRUNNER_HPP

#include "Agent.hpp"
#include "CGAgent.hpp"

#include <cassert>

template<class game_t, class agent_t>
class CGRunner {
public:
	using AgentArgs = Agent::AgentArgs;

	CGRunner(double turnLimitInMs, const AgentArgs& agentArgs) :
		turnLimitInMs(turnLimitInMs), agentArgs(agentArgs) {

	}

	void playGame() const {
		up<State> game = std::mku<game_t>();
		sp<Agent> agents[] {
			std::mksh<CGAgent>(AGENT1),
			std::mksh<agent_t>(AGENT2, firstTurnLimitInMs, game, agentArgs),
		};

		int agentCount = sizeof(agents) / sizeof(agents[0]);
		int turn = 0; 
		bool firstTurn = true;

		while (!game->isTerminal()) {
			auto& agent = agents[turn];
			sp<Action> action = agent->getAction(game);

			if (!action) {
				game = std::mku<game_t>();
				agents[0] = std::mksh<agent_t>(AGENT1, firstTurnLimitInMs, game, agentArgs);
				agents[1] = std::mksh<CGAgent>(AGENT2);
				continue;
			}
			
			if (!std::dynamic_pointer_cast<CGAgent>(agent)) {
				const auto& act = std::dynamic_pointer_cast<
					typename game_t::action_t>(action);
				assert(act);
				printAction(act);
			}
		
			for (int i = 0; i < agentCount; ++i)
				agents[i]->recordAction(action);

			if (firstTurn) {
				const auto& ptr = std::dynamic_pointer_cast<agent_t>(agents[turn]);
				if (ptr) {
					ptr->changeCalcLimit(turnLimitInMs);
					firstTurn = false;
				}
			}

			game->apply(action);
			turn ^= 1;
		}
	}

private:
	void printAction(const sp<typename game_t::action_t>& action) const {
		int gameRow = action->row, gameCol = action->col;
		int row = action->action.row, col = action->action.col;
		int posRow = gameRow * 3 + row;
		int posCol = gameCol * 3 + col;
		std::cout << posRow << " " << posCol << std::endl;
	}

private:
	double turnLimitInMs;
	AgentArgs agentArgs;
	double firstTurnLimitInMs = 1000;
};

#endif /* CGRUNNER_HPP */
