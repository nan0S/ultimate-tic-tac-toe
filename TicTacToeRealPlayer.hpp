#ifndef TICTACTOE_REAL_PLAYER_HPP
#define TICTACTOE_REAL_PLAYER_HPP

#include "Common.hpp"
#include "Player.hpp"

class TicTacToeRealPlayer : public Player {
public:
	sp<Action> getAction(const up<State>& state) override;
	std::map<std::string, std::string> getDesc() const override;
	
private:
	bool isInRange(int idx) const;
};

#endif /* TICTACTOE_REAL_PLAYER_HPP */
