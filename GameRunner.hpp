#ifndef GAME_RUNNER_HPP
#define GAME_RUNNER_HPP

#include "StatSystem.hpp"

#include <string>
#include <optional>

class GameRunner {
public:
	GameRunner(double turnLimitInMs);

	void playGames(int numberOfGames, bool verbose=false);
	void playGame(bool verbose=false, bool lastGame=false);

private:
	void announceGameStart();
	void announceGameEnd(const std::string& winner);

	StatSystem statSystem;
	double turnLimitInMs;
};

#endif /* GAME_RUNNER_HPP */
