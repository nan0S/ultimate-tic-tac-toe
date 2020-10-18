#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomAgent.hpp"
#include "FlatMCTSAgent.hpp"
#include "TicTacToeRealAgent.hpp"
#include "MCTSAgent.hpp"

GameRunner::GameRunner(double turnLimitInMs) : turnLimitInMs(turnLimitInMs) {

}

void GameRunner::playGames(int numberOfGames, bool verbose) {
	statSystem.reset();
	for (int i = 0; i < numberOfGames - 1; ++i)
		playGame(verbose);
	playGame(verbose, true);
	statSystem.showStats();
}

void GameRunner::playGame(bool verbose, bool lastGame) {
	announceGameStart();

	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		// std::mksh<RandomAgent>(AGENT1),
		// std::mksh<FlatMCTSAgent>(AGENT1, 100),
		std::mksh<MCTSAgent>(AGENT1, game, turnLimitInMs, 0.4),
		// std::mksh<RandomAgent>(AGENT2),
		std::mksh<FlatMCTSAgent>(AGENT2, 100),
		// std::mksh<MCTSAgent>(AGENT2, game, 100),
		// std::mksh<RandomAgent>(AGENT2)
		// std::mksh<FlatMCTSAgent>(AGENT1, 200),
		// std::mksh<MCTSAgent>(AGENT2, game, 200, 3.0),
	};
	int agentCount = sizeof(agents) / sizeof(agents[0]);

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

	if (lastGame) {
		for (int i = 0; i < agentCount; ++i) {
			auto id = agents[i]->getID();
			statSystem.addDesc(std::to_string(id), agents[i]->getDesc());
		}
	}

	announceGameEnd(game->getWinnerName());
}

void GameRunner::announceGameStart() {
	statSystem.recordStart();
}

void GameRunner::announceGameEnd(const std::string& winner) {
	statSystem.recordEnd(winner);
}
