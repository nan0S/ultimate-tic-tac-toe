#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomPlayer.hpp"
#include "FlatMCTSPlayer.hpp"
#include "TicTacToeRealPlayer.hpp"

void GameRunner::playGames(int numberOfGames, bool verbose) {
	statSystem.reset();
	for (int i = 0; i < numberOfGames; ++i)
		playGame(verbose);
	statSystem.showStats();
}

void GameRunner::playGame(bool verbose) {
	announceGameStart();
	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Player> players[] {
		std::mksh<FlatMCTSPlayer>(200),
		std::mksh<RandomPlayer>()
	};

	static bool firstGame = true;
	if (firstGame) {
		int playerCount = sizeof(players) / sizeof(players[0]);
		for (int i = 0; i < playerCount; ++i)
			statSystem.addDesc("PLAYER" + std::to_string(i), players[i]->getDesc());
		firstGame = false;
	}

	if (verbose)
		std::cout << *game << '\n';

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
