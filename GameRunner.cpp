#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomAgent.hpp"
#include "FlatMCTSAgent.hpp"
#include "TicTacToeRealAgent.hpp"
#include "MCTSAgent.hpp"

void GameRunner::playGames(int numberOfGames, bool verbose) {
	statSystem.reset();
	for (int i = 0; i < numberOfGames; ++i)
		playGame(verbose);
	statSystem.showStats();
}

void GameRunner::playGame(bool verbose) {
	announceGameStart();

	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		std::mksh<FlatMCTSAgent>(AGENT1, 200),
		std::mksh<MCTSAgent>(AGENT2, game, 200, 3.0),
	};
	int agentCount = sizeof(agents) / sizeof(agents[0]);

	static bool firstGame = true;
	if (firstGame) {
		for (int i = 0; i < agentCount; ++i)
			statSystem.addDesc("AGENT" + std::to_string(i + 1), agents[i]->getDesc());
		firstGame = false;
	}

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

	announceGameEnd(game->getWinnerName());
}

void GameRunner::announceGameStart() {
	statSystem.recordStart();
}

void GameRunner::announceGameEnd(const std::string& winner) {
	statSystem.recordEnd(winner);
}
