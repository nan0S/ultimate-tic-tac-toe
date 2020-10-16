#include "Common.hpp"
#include "GameRunner.hpp"
#include "CGRunner.hpp"

#include <getopt.h>

bool verboseFlag = false;
int numberOfGames = 1;

void parseArgs(int argc, char* argv[]) {
	static const char helpstr[] =
		"\nUsage: tictactoe [OPTIONS]... [TIMES]\n\n"
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
		errorExit("Usage: tictactoe [times] [saveFile]");
	if (rest >= 1)
		numberOfGames = std::stoi(argv[optind++]);
}


int main(int argc, char* argv[]) {

	std::ios_base::sync_with_stdio(false);
#ifdef LOCAL
	std::cout.tie(0);
	std::cin.tie(0);
#endif

#ifdef LOCAL
	parseArgs(argc, argv);
	GameRunner().playGames(numberOfGames, verboseFlag);
#else
	CGRunner().playGame();
#endif

	return 0;
}
