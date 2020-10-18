
#include <iostream>
#include <memory>
#include <random>
#include <chrono>

#define mksh make_shared
#define mku make_unique

template<typename T>
using sp = std::shared_ptr<T>;
template<typename T>
using up = std::unique_ptr<T>;
template<typename T>
using wp = std::weak_ptr<T>;

using KeyValue = std::pair<std::string, std::string>;

void errorExit(const std::string& msg);

namespace Random {
	extern std::mt19937 rng;

	template<typename T>
	T rand(T a, T b) {
		using dist_t = std::conditional_t<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>;
		return dist_t{a, b}(rng);
	}

	template<typename T>
	T rand(T n) {
		using dist_t = std::conditional_t<
			std::is_integral<T>::value,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>;
		if (std::is_integral<T>::value)
			--n;
		return dist_t{0, n}(rng);
	}

	template<typename T, typename Alloc, template<typename, typename> class Container>
	const T& choice(const Container<T, Alloc>& cont) {
		return cont[rand(cont.size())];
	}
}

#define PROFILING 0
#if PROFILING
#define PROFILE_SCOPE(name) SimpleTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#endif

class SimpleTimer {
public:
	SimpleTimer(const std::string& label = "unknown");
	~SimpleTimer();

private:
	static int instanceCounter;

	std::string label;
	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;

	std::string getIndent();
};

class CalcTimer {
public:
	CalcTimer(double limitInMs);

	void startCalculation();
	bool isTimeLeft() const;
	void endCalculation();

	double getAverageCalcTime() const;
	int getTotalNumberOfCals() const;
	double getLimit() const;

	void changeLimit(double newLimitInMs);

private:
	double getElapsed() const;
	
private:
	double limitInMs;
	bool isRunning = false;
	double totalCalcTime = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	int numberOfCalcs = 0;
};


#include <iomanip>
#include <cassert>

void errorExit(const std::string& msg) {
	std::cerr << "ERROR: " << msg << '\n';
	exit(EXIT_FAILURE);
}

namespace Random {
	std::mt19937 rng(std::random_device{}());
}

int SimpleTimer::instanceCounter = 0;

SimpleTimer::SimpleTimer(const std::string& label) : label(label) {
	startPoint = std::chrono::high_resolution_clock::now();
	std::cout << getIndent() << "PROFILING(" << label << ")" << '\n';
	++instanceCounter;
}

SimpleTimer::~SimpleTimer() {
	--instanceCounter;
	auto endPoint = std::chrono::high_resolution_clock::now();
	long double elapsed = std::chrono::duration_cast<
		std::chrono::nanoseconds>(endPoint - startPoint).count() * 1e-6;
	std::cout << std::fixed << std::setprecision(5);
	std::cout << getIndent() << label << ":\t" << elapsed << " ms" << '\n';
}

std::string SimpleTimer::getIndent() {
	return std::string(2 * instanceCounter, ' ');
}

CalcTimer::CalcTimer(double limitInMs) : limitInMs(limitInMs) {
}

void CalcTimer::startCalculation() {
	assert(!isRunning);
	startTime = std::chrono::high_resolution_clock::now();
	isRunning = true;
}

bool CalcTimer::isTimeLeft() const {
	return getElapsed() <= limitInMs;
}

double CalcTimer::getElapsed() const {
	auto endTime = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<
		std::chrono::nanoseconds>(endTime - startTime).count() * 1e-6;
}


void CalcTimer::endCalculation() {
	assert(isRunning);
	totalCalcTime += getElapsed();
	isRunning = false;
	++numberOfCalcs;
}

double CalcTimer::getAverageCalcTime() const {
	assert(numberOfCalcs != 0);
	return totalCalcTime / numberOfCalcs;
}

int CalcTimer::getTotalNumberOfCals() const {
	return numberOfCalcs;
}

double CalcTimer::getLimit() const {
	return limitInMs;
}

void CalcTimer::changeLimit(double newLimitInMs) {
	limitInMs = newLimitInMs;
}


struct Action {
	virtual bool equals(const sp<Action>& o) const = 0;
	virtual ~Action() = default;
};



#include <vector>

enum AgentID {
	NONE = -1, AGENT1, AGENT2
};

namespace std {
	std::string to_string(AgentID id);
}

class State;

class Agent {
public:
	Agent(AgentID id);
	AgentID getID() const;
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::vector<KeyValue> getDesc() const;
	virtual ~Agent() = default;

private:
	AgentID id;
};


#include <cassert>

namespace std {
	std::string to_string(AgentID id) {
		switch (id) {
			case NONE:
				return "NONE";
			case AGENT1:
				return "AGENT1";
			case AGENT2:
				return "AGENT2";
		}
		assert(false);
	}
}

Agent::Agent(AgentID id) : id(id) {

}

AgentID Agent::getID() const {
	return id;
}

std::vector<KeyValue> Agent::getDesc() const {
	return {};
}

void Agent::recordAction(const sp<Action>&) {

}


class State {
public:
	using reward_t = double;

	virtual bool isTerminal() const = 0;
	virtual void apply(const sp<Action>& action) = 0;
	up<State> applyCopy(const sp<Action>& action);

	virtual std::vector<sp<Action>> getValidActions() = 0;
	virtual bool isValid(const sp<Action>& action) const = 0;

	virtual up<State> clone() = 0;
	virtual bool didWin(AgentID id) const = 0;
	virtual reward_t getReward(AgentID id) const = 0;

	virtual std::ostream& print(std::ostream& out) const = 0;
	friend std::ostream& operator<<(std::ostream& out, const State& state);
	virtual std::string getWinnerName() const = 0;

	virtual ~State() = default;
};


std::ostream& operator<<(std::ostream& out, const State& state) {
	return state.print(out);
}

up<State> State::applyCopy(const sp<Action>& action) {
	auto ptr = clone();
	ptr->apply(action);
	return ptr;
}


#include <string>
#include <chrono>
#include <vector>
#include <map>

class StatSystem {
public:
	StatSystem();

	void recordStart();
	void recordEnd(const std::string& name);
	void showStats() const;
	void addDesc(const std::string& label,
			const std::vector<KeyValue>& vals);
	void reset();

private:
	void printAll() const;
	void printSeparator() const;
	void printGeneral() const;
	void printDescription() const;
	void printRecords() const;

private:
	static constexpr int LINE_SEP_LENGTH = 30;

	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
	long long accumulatedTime;

	std::map<std::string, int> counter;
	std::map<std::string, std::vector<KeyValue>> desc;

	int numberOfExps;
	bool isRunning;
};


#include <iostream>
#include <iomanip>
#include <cassert>
#include <ratio>

StatSystem::StatSystem() {
	reset();
}

void StatSystem::recordStart() {
	assert(!isRunning);

	startPoint = std::chrono::high_resolution_clock::now();
	isRunning = true;
}

void StatSystem::recordEnd(const std::string& name) {
	assert(isRunning);

	auto endPoint = std::chrono::high_resolution_clock::now();
	accumulatedTime += std::chrono::duration_cast<
		std::chrono::nanoseconds>(endPoint - startPoint).count();
	++numberOfExps;
	++counter[name];
	isRunning = false;
}

void StatSystem::showStats() const {
	assert(!isRunning);
	assert(numberOfExps != 0);
	printAll();
}

void StatSystem::printAll() const {
	printSeparator();
	printGeneral();
	printDescription();
	printRecords();
	printSeparator();
	std::cout << '\n';
}

void StatSystem::printSeparator() const {
	std::cout << '\n' << std::string(LINE_SEP_LENGTH, '-') << '\n';
}

void StatSystem::printGeneral() const {
	long double msPassed = accumulatedTime * 1e-6;
	std::cout << std::fixed << std::setprecision(2);
	std::cout << '\n';
	std::cout << "Reapeats:\t" << numberOfExps << '\n';
	std::cout << "Total time:\t" << msPassed * 0.001 << " s" << '\n';
	std::cout << std::setprecision(3);
	std::cout << "Avg time:\t" << msPassed / numberOfExps << " ms" << '\n';
}

void StatSystem::printDescription() const {
	for (const auto& [label, vals]: desc)
		if (!vals.empty()) {
			std::cout << '\n' << label << ":\n";
			for (const auto& [key, val] : vals)
				if (val == "")
					std::cout << std::string(3, ' ') << key << '\n';
				else
					std::cout << std::string(3, ' ') << key << ": " << val << '\n';
		}
}

void StatSystem::printRecords() const {
	std::cout << std::setprecision(2) << '\n';
	for (const auto& record : counter) {
		if (record.first == "")
			continue;

		double winrate = static_cast<double>(record.second) / numberOfExps * 100;
		std::cout << record.first << ":\t" << winrate << "%" << '\n';
	}
}

void StatSystem::addDesc(const std::string& label,
		const std::vector<KeyValue>& vals) {
	assert(desc.count(label) == 0);
	desc[label] = vals;
}

void StatSystem::reset() {
	accumulatedTime = 0;
	counter.clear();
	desc.clear();	
	numberOfExps = 0;
	isRunning = false;
}


class RandomAgent : public Agent {
public:
	RandomAgent(AgentID id);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
};


#include <cassert>

RandomAgent::RandomAgent(AgentID id) : Agent(id) {

}

sp<Action> RandomAgent::getAction(const up<State>& state) {
	auto validActions = state->getValidActions();
	assert(!validActions.empty());
	return Random::choice(validActions);
}

std::vector<KeyValue> RandomAgent::getDesc() const {
	return { {"Random agent with uniform distribution.", ""} };
}


#include <vector>

class FlatMCTSAgent : public Agent {
public:
	using reward_t = State::reward_t;

	FlatMCTSAgent(AgentID id, double limitInMs);

	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
	
	struct ActionStats {
		// int winCount = 0;
		reward_t reward = 0;
		int total = 0;
		bool operator<(const ActionStats& o) const;
	};

private:
	CalcTimer timer;
	std::vector<ActionStats> stats;
};



#include <cassert>
#include <algorithm>

using ActionStats = FlatMCTSAgent::ActionStats;
using reward_t = FlatMCTSAgent::reward_t;

FlatMCTSAgent::FlatMCTSAgent(AgentID id, double limitInMs) : 
	Agent(id),
	timer(limitInMs) {

}

sp<Action> FlatMCTSAgent::getAction(const up<State>& state) {
	timer.startCalculation();

	auto validActions = state->getValidActions();
	assert(!validActions.empty());

	int actionsNum = static_cast<int>(validActions.size());
	stats.resize(actionsNum);
	std::fill(stats.begin(), stats.end(), ActionStats());
	
	for (int i = 0; i < 100; ++i) {
		int randActionIdx = Random::rand(actionsNum);
		auto nState = state->applyCopy(validActions[randActionIdx]);

		while (!nState->isTerminal()) {
			auto actions = nState->getValidActions();
			const auto& action = Random::choice(actions);
			nState->apply(action);
		}

		++stats[randActionIdx].total;
		// stats[randActionIdx].reward += nState->getReward(getID());
		if (nState->didWin(getID()))
			++stats[randActionIdx].reward;
	}

	int bestIdx = std::max_element(stats.begin(), stats.end()) - stats.begin();
	const auto& bestAction = validActions[bestIdx];
	timer.endCalculation();

	return bestAction;
}

bool ActionStats::operator<(const ActionStats& o) const {
	return reward * o.total < o.reward * total;
}

std::vector<KeyValue> FlatMCTSAgent::getDesc() const {
	return { { "Flat MCTS agent.", "" },
		{ "Number of MCTS iterations", std::to_string(100) },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
	};
}


class MCTSAgent : public Agent {
public:
	using param_t = double;
	using reward_t = State::reward_t;

  	MCTSAgent(AgentID id, const up<State> &initialState,
			double limitInMs, param_t exploreSpeed=1.0);

	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	std::vector<KeyValue> getDesc() const override;

	void changeTurnLimit(double newLimitInMs);

private:
	struct MCTSNode {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		inline bool isTerminal() const;
		inline bool shouldExpand() const;
		sp<MCTSNode> expand();
		sp<MCTSNode> selectChild(param_t exploreSpeed=1.0);
		param_t UCT(const sp<MCTSNode>& v, param_t c=1.0) const;
		up<State> cloneState();
		void addReward(reward_t delta);
		sp<Action> bestAction();
		bool operator<(const MCTSNode& o) const;

		up<State> state;
		wp<MCTSNode> parent;
		std::vector<sp<MCTSNode>> children;
		std::vector<sp<Action>> actions;
		int nextActionToResolveIdx = 0;
		
		struct MCTSNodeStats {
			reward_t score = 0;
			int visits = 0;
		} stats;
	};

	sp<MCTSNode> treePolicy();
	sp<MCTSNode> expand(const sp<MCTSNode>& node);
	reward_t defaultPolicy(const sp<MCTSNode>& initialNode);
	void backup(sp<MCTSNode> node, reward_t delta);

private:
	CalcTimer timer;
	double exploreSpeed;
	sp<MCTSNode> root;
	int descended;
	int simulationCount = 0;
};


#include <cassert>
#include <algorithm>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;

MCTSAgent::MCTSAgent(AgentID id, const up<State>& initialState, 
		double limitInMs, param_t exploreSpeed) :
	Agent(id),
	timer(limitInMs),
	exploreSpeed(exploreSpeed),
	root(std::mksh<MCTSNode>(initialState)) {

}

MCTSAgent::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgent::getAction(const up<State>&) {
	timer.startCalculation();

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		int delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
		++simulationCount;
	}

	timer.endCalculation();
	return root->bestAction();	
}

sp<MCTSAgent::MCTSNode> MCTSAgent::treePolicy() {
	auto currentNode = root;
	descended = 0;
	while (!currentNode->isTerminal()) {
		++descended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = currentNode->selectChild(exploreSpeed);
	}
	return currentNode;
}

bool MCTSAgent::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgent::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSAgent::MCTSNode> MCTSAgent::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSAgent::MCTSNode> MCTSAgent::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));
	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

MCTSAgent::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<MCTSAgent::MCTSNode> MCTSAgent::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSAgent::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

reward_t MCTSAgent::defaultPolicy(const sp<MCTSNode>& initialNode) {
	auto state = initialNode->cloneState();

     while (!state->isTerminal()) {
		auto actions = state->getValidActions();
		const auto& action = Random::choice(actions);
		state->apply(action);
	}

	return state->getReward(getID());
	// return state->didWin(getID());
}

up<State> MCTSAgent::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgent::backup(sp<MCTSNode> node, reward_t delta) {
	int ascended = 0;
	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++ascended;
	}
	assert(descended + 1 == ascended);
}

void MCTSAgent::MCTSNode::addReward(reward_t delta) {
	stats.score += delta;
	++stats.visits;
}

sp<Action> MCTSAgent::MCTSNode::bestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

bool MCTSAgent::MCTSNode::operator<(const MCTSNode& o) const {
	// return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
	return stats.visits < o.stats.visits;
}

void MCTSAgent::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
			[&action](const auto& x){
		return action->equals(x);
	}) - root->actions.begin();

	assert(recordActionIdx < int(root->actions.size()));
	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = std::mksh<MCTSNode>(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

std::vector<KeyValue> MCTSAgent::getDesc() const {
	auto averageSimulationCount = double(simulationCount) / timer.getTotalNumberOfCals();
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "Number of iterations", std::to_string(100) },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreSpeed) }
	};
}

void MCTSAgent::changeTurnLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}

#include <iostream>
#include <vector>


class TicTacToe {
public:
	TicTacToe();

	struct TicTacToeAction {
		int row, col;
		TicTacToeAction(int row, int col);
	};

	bool isTerminal() const;
	void apply(AgentID turn, const TicTacToeAction& action);

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int row) const;

	AgentID getWinner() const;

	bool isLegal(const TicTacToeAction& action) const;
	bool isEmpty(int i, int j) const;

private:
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board Size has to be positive");
	AgentID board[BOARD_SIZE][BOARD_SIZE];

	int emptyCells = BOARD_SIZE * BOARD_SIZE;

	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	inline bool isInRange(int idx) const;

	inline char getCharAt(int row, int col) const;
	inline char getCharAtTerminal(int row, int col) const;
	inline char convertSymbolAt(int row, int col) const;

	inline bool isOnDiag1(int row, int col) const;
	inline bool isOnDiag2(int row, int col) const;
	inline bool isOnSide(int row, int col) const;
	inline bool isOnTop(int row, int col) const;
};



#include <iostream>
#include <algorithm>
#include <cassert>

#undef PROFILING
#define PROFILING 0

using TicTacToeAction = TicTacToe::TicTacToeAction;

TicTacToe::TicTacToe() {
	PROFILE_FUNCTION();
	for (int i = 0; i < BOARD_SIZE; ++i)
		std::fill(board[i], board[i] + BOARD_SIZE, NONE);
}

TicTacToe::TicTacToeAction::TicTacToeAction(int row, int col) : row(row), col(col) {

}

bool TicTacToe::isTerminal() const {
	PROFILE_FUNCTION();

	if (emptyCells == 0)
		return true;
	for (int i = 0; i < BOARD_SIZE; ++i)
		if (isRowDone(i) || isColDone(i))
			return true;
	return isDiag1Done() || isDiag2Done();
}

void TicTacToe::apply(AgentID turn, const TicTacToeAction& action) {
	PROFILE_FUNCTION();

	assert(isLegal(action));
	board[action.row][action.col] = turn;
	--emptyCells;
}

bool TicTacToe::isRowDone(int row) const {
	PROFILE_FUNCTION();

	AgentID val =  board[row][0];
	return val != NONE && 
		std::all_of(board[row], board[row] + BOARD_SIZE,
				  [&val](const AgentID& p){ return p == val; });
}

bool TicTacToe::isColDone(int col) const {
	PROFILE_FUNCTION();

	AgentID val =  board[0][col];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][col] != val)
			return false;
	return true;
}

bool TicTacToe::isDiag1Done() const {
	PROFILE_FUNCTION();

	AgentID val = board[0][0];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][i] != val)
			return false;
	return true;
}

bool TicTacToe::isDiag2Done() const {
	PROFILE_FUNCTION();

	AgentID val = board[0][BOARD_SIZE - 1];
	if (val == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][BOARD_SIZE - 1 - i] != val)
			return false;
	return true;
}

bool TicTacToe::isEmpty(int i, int j) const {
	PROFILE_FUNCTION();

	return board[i][j] == NONE;
}

bool TicTacToe::isLegal(const TicTacToeAction& action) const {
	PROFILE_FUNCTION();

	return isInRange(action.row) && 
		  isInRange(action.col) && 
		  isEmpty(action.row, action.col);
}

bool TicTacToe::isInRange(int idx) const {
	PROFILE_FUNCTION();

	return 0 <= idx && idx < BOARD_SIZE;
}

AgentID TicTacToe::getWinner() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (isRowDone(i))
			return board[i][0];
		if (isColDone(i))
			return board[0][i];
	}
	if (isDiag1Done())
		return board[0][0];
	if (isDiag2Done())
		return board[0][BOARD_SIZE - 1];
	return NONE;
}

void TicTacToe::printLineSep(std::ostream& out) const {
	for (int i = 0; i < BOARD_SIZE; ++i)
		out << "+---";
	out << "+ ";
}

void TicTacToe::printRow(std::ostream& out, int row) const {
	for (int j = 0; j < BOARD_SIZE; ++j)
		out << "| " << getCharAt(row, j) << " ";
	out << "| ";
}

char TicTacToe::getCharAt(int row, int col) const {
	if (isTerminal())
		return getCharAtTerminal(row, col);
	return convertSymbolAt(row, col);

}

char TicTacToe::getCharAtTerminal(int row, int col) const {
	auto winner = getWinner();
	switch (winner) {
		case AGENT1:
			return isOnDiag1(row, col) || 
				  isOnDiag2(row, col) ?
				  'X' : ' ';
		case AGENT2:
			return isOnSide(row, col) ||
				  isOnTop(row, col) ?
				  'O' : ' ';
		case NONE:
			return convertSymbolAt(row, col);
	}
	assert(false);
}

char TicTacToe::convertSymbolAt(int row, int col) const {
	const auto symbol = board[row][col];
	switch (symbol) {
		case AGENT1:
			return 'X';
		case AGENT2:
			return 'O';
		case NONE:
			return ' ';
	}
	assert(false);
}

bool TicTacToe::isOnDiag1(int row, int col) const {
	return row == col;
}

bool TicTacToe::isOnDiag2(int row, int col) const {
	return row + col == BOARD_SIZE - 1;
}

bool TicTacToe::isOnSide(int, int col) const {
	return col == 0 || col == BOARD_SIZE - 1;
}

bool TicTacToe::isOnTop(int row, int) const {
	return row == 0 || row == BOARD_SIZE - 1;
}


class UltimateTicTacToe : public State {
public:
	using reward_t = State::reward_t;

	struct UltimateTicTacToeAction : public Action {
		UltimateTicTacToeAction(const AgentID& agentID, int row, int col,
				const TicTacToe::TicTacToeAction& action);
		bool equals(const sp<Action>& o) const override;

		AgentID agentID;
		int row, col;
		TicTacToe::TicTacToeAction action;
	};

	bool isTerminal() const override;
	void apply(const sp<Action>& act) override;

	std::vector<sp<Action>> getValidActions() override;
	bool isValid(const sp<Action>& act) const override;

	up<State> clone() override;
	bool didWin(AgentID id) const override; 
	reward_t getReward(AgentID id) const override;

	std::ostream& print(std::ostream& out) const override;
	std::string getWinnerName() const override;

	bool isLegal(const sp<UltimateTicTacToeAction>& action) const;
	
	static constexpr int BOARD_SIZE = 3;
	static_assert(BOARD_SIZE > 0, "Board size has to be positive");

private:
	TicTacToe board[BOARD_SIZE][BOARD_SIZE];
	AgentID turn = AGENT1;
	int lastRow = -1, lastCol = -1;

	bool isAllTerminal() const;
	bool isRowDone(int row) const;
	bool isColDone(int col) const;
	bool isDiag1Done() const;
	bool isDiag2Done() const;

	bool isInRange(int idx) const;
	bool canMove(AgentID agentID) const;
	bool properBoard(int boardRow, int boardCol) const;

	AgentID getWinner() const;

	void printLineSep(std::ostream& out) const;
	void printRow(std::ostream& out, int i) const;
};


#include <algorithm>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
using reward_t = UltimateTicTacToe::reward_t;

UltimateTicTacToeAction::UltimateTicTacToeAction(const AgentID& agentID, int row, int col,
		const TicTacToe::TicTacToeAction& action) :
	agentID(agentID), row(row), col(col), action(action) {
	PROFILE_FUNCTION();
}
						
bool UltimateTicTacToe::isTerminal() const {
	PROFILE_FUNCTION();

	if (isAllTerminal())
		return true;
	for (int i = 0; i < BOARD_SIZE; ++i)
		if (isRowDone(i) || isColDone(i))
			return true;
	return isDiag1Done() || isDiag2Done();
}

bool UltimateTicTacToe::isAllTerminal() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i)
		if (!std::all_of(board[i], board[i] + BOARD_SIZE,
					[](const auto& x){ return x.isTerminal(); }))
			return false;
	return true;
}

bool UltimateTicTacToe::isRowDone(int row) const {
	PROFILE_FUNCTION();

	const auto winner = board[row][0].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[row][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isColDone(int col) const {
	PROFILE_FUNCTION();

	const auto winner = board[0][col].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][col].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag1Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][0].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][i].getWinner() != winner)
			return false;
	return true;
}

bool UltimateTicTacToe::isDiag2Done() const {
	PROFILE_FUNCTION();

	const auto winner = board[0][BOARD_SIZE - 1].getWinner();
	if (winner == NONE)
		return false;
	for (int i = 1; i < BOARD_SIZE; ++i)
		if (board[i][BOARD_SIZE - 1 - i].getWinner() != winner)
			return false;
	return true;
}

void UltimateTicTacToe::apply(const sp<Action>& act) {
	PROFILE_FUNCTION();

	const auto& action = std::dynamic_pointer_cast<UltimateTicTacToeAction>(act);
	assert(action);
	assert(isLegal(action));

	board[action->row][action->col].apply(turn, action->action);
	turn = turn == AGENT1 ? AGENT2 : AGENT1;
	
	if (board[action->action.row][action->action.col].isTerminal())
		lastRow = lastCol = -1;
	else
		lastRow = action->action.row,
		lastCol = action->action.col;
}

bool UltimateTicTacToe::isLegal(const sp<UltimateTicTacToeAction>& action) const {
	PROFILE_FUNCTION();

	return canMove(action->agentID) &&
		  isInRange(action->row) &&
		  isInRange(action->col) &&
		  properBoard(action->row, action->col) && 
		  board[action->row][action->col].isLegal(action->action);
}

bool UltimateTicTacToe::canMove(AgentID agentID) const {
	return agentID == NONE || agentID == turn;
}

bool UltimateTicTacToe::isInRange(int idx) const {
	PROFILE_FUNCTION();

	return 0 <= idx && idx < BOARD_SIZE;
}

bool UltimateTicTacToe::properBoard(int boardRow, int boardCol) const {
	return (lastRow == -1 && lastCol == -1) || 
		(boardRow == lastRow && boardCol == lastCol);
}

std::vector<sp<Action>> UltimateTicTacToe::getValidActions() {
	PROFILE_FUNCTION();

	std::vector<sp<Action>> validActions;

	if (lastRow == -1 && lastCol == -1) {
		for (int i = 0; i < BOARD_SIZE; ++i)
			for (int j = 0; j < BOARD_SIZE; ++j) {
				const auto& cell = board[i][j];
				if (cell.isTerminal())
					continue;
				for (int k = 0; k < BOARD_SIZE; ++k)
					for (int l = 0; l < BOARD_SIZE; ++l)
						if (cell.isEmpty(k, l))
							validActions.push_back(std::mksh<UltimateTicTacToeAction>(
								NONE, i, j, TicTacToe::TicTacToeAction(k, l)));
			}
	}
	else {
		const auto& cell = board[lastRow][lastCol];
		if (cell.isTerminal())
			return {};
		assert(!cell.isTerminal());
		for (int i = 0; i < BOARD_SIZE; ++i)
			for (int j = 0; j < BOARD_SIZE; ++j)
				if (cell.isEmpty(i, j))
					validActions.push_back(std::mksh<UltimateTicTacToeAction>(
						NONE, lastRow, lastCol, TicTacToe::TicTacToeAction(i, j)));
	}

	return validActions;
}

bool UltimateTicTacToe::isValid(const sp<Action>& act) const {
	const auto& action = std::dynamic_pointer_cast<UltimateTicTacToeAction>(act);
	assert(action);
	return isLegal(action);
}

bool UltimateTicTacToe::didWin(AgentID id) const {
	return id == getWinner();
}

reward_t UltimateTicTacToe::getReward(AgentID id) const {
	auto winner = getWinner();
	if (winner == NONE)
		return 0.5;
	return id == winner ? 1 : 0;
}

up<State> UltimateTicTacToe::clone() {
	return up<State>(new UltimateTicTacToe(*this));
}

AgentID UltimateTicTacToe::getWinner() const {
	PROFILE_FUNCTION();

	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (isRowDone(i))
			return board[i][0].getWinner();
		if (isColDone(i))
			return board[0][i].getWinner();
	}
	if (isDiag1Done())
		return board[0][0].getWinner();
	if (isDiag2Done())
		return board[0][BOARD_SIZE - 1].getWinner();
	assert(isAllTerminal());
	return NONE; 
}

std::ostream& UltimateTicTacToe::print(std::ostream& out) const {
	for (int i = 0; i < BOARD_SIZE; ++i) {
		printLineSep(out);
		for (int row = 0; row < BOARD_SIZE; ++row) {
			for (int j = 0; j < BOARD_SIZE; ++j) {
				out << "| ";
				board[i][j].printLineSep(out);
			}
			out << "|\n";
			for (int j = 0; j < BOARD_SIZE; ++j) {
				out << "| ";
				board[i][j].printRow(out, row);
			}
			out << "|\n";
		}
		for (int j = 0; j < BOARD_SIZE; ++j) {
			out << "| ";
			board[i][j].printLineSep(out);
		}
		out << "|\n";
	}
	printLineSep(out);
	return out;
}

void UltimateTicTacToe::printLineSep(std::ostream& out) const {
	std::string sep((4 * BOARD_SIZE + 1) + 2, '-');
	for (int j = 0; j < BOARD_SIZE; ++j)
		out << '+' << sep;
	out << "+\n";
}

std::string UltimateTicTacToe::getWinnerName() const {
	PROFILE_FUNCTION();

	assert(isTerminal());
	const auto winner = getWinner();
	switch (winner) {
		case AGENT1:
			return "AGENT1 / Player X";
		case AGENT2:
			return "AGENT2 / Player O";
		case NONE:
			return "";
	}
	assert(false);
}

bool UltimateTicTacToeAction::equals(const sp<Action>& o) const {
	auto oo = std::dynamic_pointer_cast<UltimateTicTacToeAction>(o);
	assert(oo);
	return row == oo->row && col == oo->col &&
		action.row == oo->action.row && action.col == oo->action.col;
}


class TicTacToeRealAgent : public Agent {
public:
	TicTacToeRealAgent(AgentID id);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
	
private:
	bool isInRange(int idx) const;
};



#include <memory>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
using TicTacToeAction = TicTacToe::TicTacToeAction; 

TicTacToeRealAgent::TicTacToeRealAgent(AgentID id) : Agent(id) {

}

sp<Action> TicTacToeRealAgent::getAction(const up<State>& state) {
	while (true) {
		std::cout << "Type TicTacToe game coords (row, col): "; 
		std::cout.flush();
		int gameRow, gameCol;
		std::cin >> gameRow >> gameCol;

		assert(isInRange(gameRow));
		assert(isInRange(gameCol));

		std::cout << "Type TicTacToe board coords (row, col): "; 
		std::cout.flush();
		int row, col;
		std::cin >> row >> col;

		assert(isInRange(row));
		assert(isInRange(col));

		auto action = std::mksh<UltimateTicTacToeAction>(
			getID(), gameRow - 1, gameCol - 1,
			TicTacToeAction(row - 1, col - 1));

		if (state->isValid(action))
			std::cout << "You typed invalid action. Try again.\n";
		else
			return action;
	}
	assert(false);
}

bool TicTacToeRealAgent::isInRange(int idx) const {
	return 1 <= idx && idx <= UltimateTicTacToe::BOARD_SIZE;
}

std::vector<KeyValue> TicTacToeRealAgent::getDesc() const {
	return { { "Real world, interactive player.", "" } };
}


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



GameRunner::GameRunner(double turnLimitInMs) : turnLimitInMs(turnLimitInMs) {

}

void GameRunner::playGames(int numberOfGames, bool verbose) {
	statSystem.reset();
	for (int i = 0; i < numberOfGames - 1; ++i)
		playGame(verbose);
	playGame(verbose, true);
	statSystem.showStats();
}

void GameRunner::playGame(bool verbose, bool lastGame) {
	announceGameStart();

	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		std::mksh<MCTSAgent>(AGENT1, game, turnLimitInMs, 0.4),
		std::mksh<FlatMCTSAgent>(AGENT2, 100),
	};
	int agentCount = sizeof(agents) / sizeof(agents[0]);

	if (verbose)
		std::cout << *game << '\n';

	int turn = 0; 
	while (!game->isTerminal()) {
		auto& agent = agents[turn];
		sp<Action> action = agent->getAction(game);
	
		for (int i = 0; i < agentCount; ++i)
			agents[i]->recordAction(action);

		game->apply(action);
		turn ^= 1;

		if (verbose)
			std::cout << *game << '\n';
	}

	if (lastGame) {
		for (int i = 0; i < agentCount; ++i) {
			auto id = agents[i]->getID();
			statSystem.addDesc(std::to_string(id), agents[i]->getDesc());
		}
	}

	announceGameEnd(game->getWinnerName());
}

void GameRunner::announceGameStart() {
	statSystem.recordStart();
}

void GameRunner::announceGameEnd(const std::string& winner) {
	statSystem.recordEnd(winner);
}


class CGAgent : public Agent {
public:
	CGAgent(AgentID id);
	sp<Action> getAction(const up<State>& state) override;
};


#include <iostream>

CGAgent::CGAgent(AgentID id) : Agent(id) {

}

sp<Action> CGAgent::getAction(const up<State>&) {
	int oppRow, oppCol;
	std::cin >> oppRow >> oppCol; std::cin.ignore();
	
	int validActionNum;
	std::cin >> validActionNum; std::cin.ignore();
	for (int i = 0; i < validActionNum; ++i) {
		int row, col;
		std::cin >> row >> col; std::cin.ignore();
	}

	if (oppRow == -1 && oppCol == -1)
		return {};

	int gameRow = oppRow / 3, gameCol = oppCol / 3;
	int row = oppRow % 3, col = oppCol % 3;
	
	return std::mksh<UltimateTicTacToe::UltimateTicTacToeAction>(
		getID(), gameRow, gameCol, TicTacToe::TicTacToeAction(row, col));
}


class CGRunner {
public:
	CGRunner(double turnLimitInMs);

	void playGame() const;

private:
	void printAction(const sp<UltimateTicTacToe::UltimateTicTacToeAction>& action) const;

private:
	double turnLimitInMs;
	double firstTurnLimitInMs = 1000;
};



#include <cassert>

CGRunner::CGRunner(double turnLimitInMs) : turnLimitInMs(turnLimitInMs) {

}

void CGRunner::playGame() const {
	up<State> game = std::mku<UltimateTicTacToe>();
	sp<Agent> agents[] {
		std::mksh<CGAgent>(AGENT1),
		std::mksh<MCTSAgent>(AGENT2, game, firstTurnLimitInMs, 0.4),
	};

	int agentCount = sizeof(agents) / sizeof(agents[0]);
	int turn = 0; 
	bool firstTurn = true;
	while (!game->isTerminal()) {
		auto& agent = agents[turn];
		sp<Action> action = agent->getAction(game);

		if (!action) {
			game = std::mku<UltimateTicTacToe>();
			agents[0] = std::mksh<MCTSAgent>(AGENT1, game, firstTurnLimitInMs, 0.4),
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
			const auto& ptr = std::dynamic_pointer_cast<MCTSAgent>(agents[turn]);
			if (ptr) {
				ptr->changeTurnLimit(turnLimitInMs);
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

#include <getopt.h>

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
	GameRunner(turnLimitInMs).playGames(numberOfGames, verboseFlag);
#else
	CGRunner(turnLimitInMs).playGame();
#endif

	return 0;
}