#ifndef GAME_RUNNER_HPP
#define GAME_RUNNER_HPP

#include "StatSystem.hpp"

#include <string>

class GameRunner {
public:
	void playGames(int numberOfGames, bool verbose=false);
	void playGame(bool verbose=false);

private:
	void announceGameStart();
	void announceGameEnd(const std::string& winner);

	StatSystem statSystem;
};

#endif /* GAME_RUNNER_HPP */
