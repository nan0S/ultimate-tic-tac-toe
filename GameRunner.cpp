#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomPlayer.hpp"

void GameRunner::playGames(int numberOfGames, bool verbose) {
	statSystem.reset();
	for (int i = 0; i < numberOfGames; ++i)
		playGame(verbose);
	statSystem.showStats();
}

void GameRunner::playGame(bool verbose) {
	announceGameStart();
	sp<State> game = std::mksh<UltimateTicTacToe>();
	sp<Player> players[] {
		std::mksh<RandomPlayer>(),
		std::mksh<RandomPlayer>()
	};

	int turn = 0; 
	while (!game->isTerminal()) {
		auto& player = players[turn];
		sp<Action> action = player->getAction(game);
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
