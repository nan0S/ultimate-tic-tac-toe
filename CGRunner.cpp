#include "CGRunner.hpp"

#include "UltimateTicTacToe.hpp"
#include "MCTSAgent.hpp"
#include "CGAgent.hpp"
#include "RandomAgent.hpp"
#include "MCTSAgentWithMAST.hpp"

#include <cassert>

CGRunner::CGRunner(double turnLimitInMs) : turnLimitInMs(turnLimitInMs) {

}

void CGRunner::playGame() const {
	using agent_t = MCTSAgent;

	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		std::mksh<CGAgent>(AGENT1),
		std::mksh<agent_t>(AGENT2, game, firstTurnLimitInMs, 0.4),
	};

	int agentCount = sizeof(agents) / sizeof(agents[0]);
	int turn = 0; 
	bool firstTurn = true;
	while (!game->isTerminal()) {
		auto& agent = agents[turn];
		sp<Action> action = agent->getAction(game);

		if (!action) {
			game = std::mku<UltimateTicTacToe>();
			agents[0] = std::mksh<agent_t>(AGENT1, game, firstTurnLimitInMs, 0.4),
			agents[1] = std::mksh<CGAgent>(AGENT2);
			continue;
		}
		
		if (!std::dynamic_pointer_cast<CGAgent>(agent)) {
			const auto& act = std::dynamic_pointer_cast<UltimateTicTacToe::UltimateTicTacToeAction>(action);
			assert(act);
			printAction(act);
		}
	
		for (int i = 0; i < agentCount; ++i)
			agents[i]->recordAction(action);

		if (firstTurn) {
			const auto& ptr = std::dynamic_pointer_cast<agent_t>(agents[turn]);
			if (ptr) {
				ptr->changeCalcLimit(turnLimitInMs);
				firstTurn = false;
			}
		}

		game->apply(action);
		turn ^= 1;
	}
}

void CGRunner::printAction(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const {
	int gameRow = action->row, gameCol = action->col;
	int row = action->action.row, col = action->action.col;
	int posRow = gameRow * 3 + row;
	int posCol = gameCol * 3 + col;
	std::cout << posRow << " " << posCol << std::endl;
}
