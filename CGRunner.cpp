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
}

void CGRunner::printAction(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const {
}
