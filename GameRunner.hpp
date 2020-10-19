#ifndef GAME_RUNNER_HPP
#define GAME_RUNNER_HPP

#include "State.hpp"
#include "Agent.hpp"
#include "StatSystem.hpp"

#include <string>
#include <map>

template<class game_t, class agent1_t, class agent2_t>
class GameRunner {
public:
	using AgentArgs = Agent::AgentArgs;

	GameRunner(double turnLimitInMs, const AgentArgs& agent1Args, const AgentArgs& agent2Args) :
		turnLimitInMs(turnLimitInMs), agent1Args(agent1Args), agent2Args(agent2Args) {
		
	}

	void playGames(int numberOfGames, bool verbose=false) {
		statSystem.reset();
		for (int i = 0; i < numberOfGames - 1; ++i)
			playGame(verbose);
		playGame(verbose, true);
		statSystem.showStats();
	}

	void playGame(bool verbose=false, bool lastGame=false) {
		announceGameStart();

		up<State> game = std::mku<game_t>();
		sp<Agent> agents[] {
			std::mksh<agent1_t>(AGENT1, game, turnLimitInMs, agent1Args),
			std::mksh<agent2_t>(AGENT2, game, turnLimitInMs, agent2Args)
		};

		if (verbose)
			std::cout << *game << '\n';

		int turn = 0; 
		while (!game->isTerminal()) {
			auto& agent = agents[turn];
			sp<Action> action = agent->getAction(game);
			for (int i = 0; i < agentCount; ++i)
				agents[i]->recordAction(action);

			game->apply(action);
			turn ^= 1;
			if (verbose)
				std::cout << *game << '\n';
		}

		if (lastGame)
			for (int i = 0; i < agentCount; ++i) {
				auto id = agents[i]->getID();
				statSystem.addDesc(std::to_string(id), agents[i]->getDesc());
			}

		announceGameEnd(game->getWinnerName());
	}

private:
	void announceGameStart() {
		statSystem.recordStart();
	}

	void announceGameEnd(const std::string& winner) {
		statSystem.recordEnd(winner);
	}

private:
	const int agentCount = 2;
	double turnLimitInMs;
	AgentArgs agent1Args;
	AgentArgs agent2Args;
	StatSystem statSystem;
};

#endif /* GAME_RUNNER_HPP */
