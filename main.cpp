#include "Common.hpp"
#include "GameRunner.hpp"
#include "CGRunner.hpp"
#include "UltimateTicTacToe.hpp"
#include "RandomAgent.hpp"
#include "FlatMCTSAgent.hpp"
#include "MCTSAgent.hpp"
#include "MCTSAgentWithMAST.hpp"

#include <getopt.h>
#include <fstream>
#include <algorithm>

bool verboseFlag = false;
int numberOfGames = 1;
double turnLimitInMs = 100;

void parseArgs(int argc, char* argv[]) {
	static const char helpstr[] =
		"\nUsage: tictactoe [OPTIONS]... [TIMES] [TURN_LIMIT_IN_MS]\n\n"
		"Run TIMES TicTacToe games.\n\n"
		"List of possible options:\n"
		"\t-v, --verbose\tprint the game\n"
		"\t-h, --help\tprint this help\n\n";

	static option longopts[] {
		{"verbose", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'}
	};

	int idx, opt;
	while ((opt = getopt_long(argc, argv, "vh", longopts, &idx)) != -1) {
		switch (opt) {
			case 'v':
				verboseFlag = true;
				break;
			case 'h':
				std::cout << helpstr;
				exit(EXIT_SUCCESS);
		}
	}

	int rest = argc - optind;
	if (rest > 2)
		errorExit("Usage: tictactoe [times] [turnLimitInMS]");
	if (rest > 0)
		numberOfGames = std::stoi(argv[optind++]);
	if (rest > 1)
		turnLimitInMs = std::stold(argv[optind++]);
}

int main(int argc, char* argv[]) {

	std::ios_base::sync_with_stdio(false);

#ifdef LOCAL
	parseArgs(argc, argv);
	auto gameRunner = GameRunner<UltimateTicTacToe, MCTSAgentWithMAST, MCTSAgent>(
		turnLimitInMs, {
			{ "exploreFactor", 0.4 },
			{ "epsilon", 0.8 },
			{ "decayFactor", 0.6 }
		},
		{ { "exploreFactor", 0.4 } }
	);
	gameRunner.playGames(numberOfGames, verboseFlag);
#else
	auto cgRunner = CGRunner<UltimateTicTacToe, MCTSAgentWithMAST>(
		turnLimitInMs, {
			{ "exploreFactor", 0.4 },
			{ "epsilon", 0.8 },
			{ "decayFactor", 0.6 }
		}
	);
	cgRunner.playGame();
#endif

	return 0;
}
