#include "GameRunner.hpp"

#include "Common.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomAgent.hpp"
#include "FlatMCTSAgent.hpp"
#include "TicTacToeRealAgent.hpp"
#include "MCTSAgent.hpp"
#include "MCTSAgentWithMAST.hpp"

GameRunner::GameRunner(double turnLimitInMs,
	std::map<std::string, double> args) :
	turnLimitInMs(turnLimitInMs), args(args) {

}

void GameRunner::playGames(int numberOfGames, bool verbose) {
}

int GameRunner::playGame(bool verbose, bool lastGame) {
}

void GameRunner::announceGameStart() {
	statSystem.recordStart();
}

void GameRunner::announceGameEnd(const std::string& winner) {
	statSystem.recordEnd(winner);
}
