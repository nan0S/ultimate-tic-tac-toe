#include "CGRunner.hpp"

#include "UltimateTicTacToe.hpp"
#include "MCTSAgent.hpp"
#include "CGAgent.hpp"

#include <cassert>

void CGRunner::playGame() const {
	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		std::mksh<CGAgent>(AGENT1),
		std::mksh<MCTSAgent>(AGENT2, game, 200, 2.0),
	};
	int agentCount = sizeof(agents) / sizeof(agents[0]);

	// std::cout << *game << std::endl;

	int turn = 0; 
	while (!game->isTerminal()) {
		auto& agent = agents[turn];
		sp<Action> action = agent->getAction(game);

		if (!action) {
			game = std::mku<UltimateTicTacToe>();
			agents[0] = std::mksh<MCTSAgent>(AGENT1, game, 200, 2.0);
			agents[1] = std::mksh<CGAgent>(AGENT2);
			continue;
		}
		
		const auto& act = std::dynamic_pointer_cast<UltimateTicTacToe::UltimateTicTacToeAction>(action);
		assert(act);
		print(act);
	
		for (int i = 0; i < agentCount; ++i)
			agents[i]->recordAction(action);

		game->apply(action);
		// std::cout << *game  << std::endl;
		turn ^= 1;
	}
}

void CGRunner::print(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const {
	int gameRow = action->row, gameCol = action->col;
	int row = action->action.row, col = action->action.col;
	int posRow = gameRow * 3 + row;
	int posCol = gameCol * 3 + col;
	std::cout << posRow << " " << posCol << std::endl;
}
