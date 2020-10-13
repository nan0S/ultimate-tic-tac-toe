#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomPlayer.hpp"

#define NDEBUG

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
		std::mksh<RandomPlayer>(),
		std::mksh<RandomPlayer>()
	};

	// using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
	// using TicTacToeAction = TicTacToe::TicTacToeAction;
	// auto action1 = std::mksh<UltimateTicTacToeAction>(
		// 1, 2, TicTacToeAction(2, 2));
	// auto action2 = std::mksh<UltimateTicTacToeAction>(
		// 2, 2, TicTacToeAction(1, 2));
	// std::cout << *game << '\n';
	// auto game2 = game->applyCopy(action1);
	// std::cout << *game << '\n';
	// std::cout << *game2 << '\n';
	// game2->apply(action2);
	// game->apply(action1);
	// std::cout << *game << '\n';
	// std::cout << *game2 << '\n';
	// return;

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
