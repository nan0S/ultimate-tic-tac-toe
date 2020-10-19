
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
	void stopCalculation();

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


void CalcTimer::stopCalculation() {
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
	virtual int getIdx() const = 0;
	virtual ~Action() = default;
};



#include <vector>
#include <map>

enum AgentID {
	NONE = -1, AGENT1, AGENT2
};

namespace std {
	std::string to_string(AgentID id);
}

class State;

class Agent {
public:
	using param_t = double;
	using AgentArgs = std::map<std::string, param_t>;

	Agent(AgentID id);
	AgentID getID() const;
	virtual sp<Action> getAction(const up<State>& state) = 0;
	virtual void recordAction(const sp<Action>& action);
	virtual std::vector<KeyValue> getDesc() const;
	virtual param_t getOrDefault(const AgentArgs& args, const std::string& key, 
		param_t defaultVal) const;
	virtual ~Agent() = default;

protected:
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

using param_t = Agent::param_t;

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

param_t Agent::getOrDefault(const AgentArgs& args, const std::string& key,
	param_t defaultVal) const {
	return args.count(key) ? args.at(key) : defaultVal;
}


class State {
public:
	using reward_t = double;

	virtual bool isTerminal() const = 0;
	virtual void apply(const sp<Action>& action) = 0;
	up<State> applyCopy(const sp<Action>& action);

	virtual constexpr int getAgentCount() const = 0;
	virtual constexpr int getActionCount() const = 0;

	virtual std::vector<sp<Action>> getValidActions() = 0;
	virtual bool isValid(const sp<Action>& action) const = 0;

	virtual up<State> clone() = 0;
	virtual bool didWin(AgentID id) const = 0;
	virtual reward_t getReward(AgentID id) const = 0;
	virtual AgentID getTurn() const = 0;

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
	RandomAgent(AgentID id, const up<State>&, double, const AgentArgs&);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
};


#include <cassert>

RandomAgent::RandomAgent(AgentID id, const up<State>&, double, const AgentArgs&) : Agent(id) {

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

	FlatMCTSAgent(AgentID id, const up<State>&, double limitInMs, const AgentArgs&);

	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
	
	struct ActionStats {
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

FlatMCTSAgent::FlatMCTSAgent(AgentID id, const up<State>&, double limitInMs, const AgentArgs&) : 
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
	timer.stopCalculation();

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


class MCTSAgentBase : public Agent {
public:
	using param_t = Agent::param_t;
	using reward_t = State::reward_t;

protected:
	struct MCTSNode {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		bool isTerminal() const;
		bool shouldExpand() const;
		sp<MCTSNode> expand();
		sp<MCTSNode> selectChild(param_t exploreFactor=1.0);

		virtual sp<MCTSNode> getChildFromState(up<State>&& state) = 0;

		param_t UCT(const sp<MCTSNode>& v, param_t c=1.0) const;
		void addReward(reward_t delta);
		sp<Action> getBestAction();
		up<State> cloneState();
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

public:
  	MCTSAgentBase(AgentID id, up<MCTSNode>&& root,
		double calcLimitInMs, const AgentArgs& args);

	sp<Action> getAction(const up<State> &state) override;
	void recordAction(const sp<Action> &action) override;
	void changeCalcLimit(double newLimitInMs);

protected:
	virtual sp<MCTSNode> treePolicy() = 0;
	sp<MCTSNode> expand(const sp<MCTSNode>& node);
	virtual reward_t defaultPolicy(const sp<MCTSNode>& initialNode) = 0;
	virtual void backup(sp<MCTSNode> node, reward_t delta) = 0;
	virtual void postWork();

protected:
	sp<MCTSNode> root;
	CalcTimer timer;
	double exploreFactor;

	int descended;
	int simulationCount = 0;
};


#include <cassert>
#include <algorithm>

using param_t = MCTSAgentBase::param_t;
using reward_t = MCTSAgentBase::reward_t;

MCTSAgentBase::MCTSAgentBase(AgentID id, up<MCTSAgentBase::MCTSNode>&& root, 
		double calcLimitInMs, const AgentArgs& args) :
	Agent(id),
	root(std::move(root)),
	timer(calcLimitInMs),
	exploreFactor(getOrDefault(args, "exploreFactor", 0.4)) {

}

MCTSAgentBase::MCTSNode::MCTSNode(const up<State>& initialState)
	: state(initialState->clone()), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

MCTSAgentBase::MCTSNode::MCTSNode(up<State>&& initialState)
	: state(std::move(initialState)), actions(state->getValidActions()) {
	std::shuffle(actions.begin(), actions.end(), Random::rng);
}

sp<Action> MCTSAgentBase::getAction(const up<State>&) {
	timer.startCalculation();

	while (timer.isTimeLeft()) {
		auto selectedNode = treePolicy();
		reward_t delta = defaultPolicy(selectedNode);
		backup(selectedNode, delta);
		++simulationCount;
	}

	const auto result = root->getBestAction();
	postWork();
	timer.stopCalculation();

	return result;
}

bool MCTSAgentBase::MCTSNode::isTerminal() const {
	return state->isTerminal();
}

bool MCTSAgentBase::MCTSNode::shouldExpand() const {
	return nextActionToResolveIdx < int(actions.size());
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::expand(const sp<MCTSNode>& node) {
	auto newNode = node->expand();
	newNode->parent = node;
	return newNode;
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::MCTSNode::expand() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx++];
	children.push_back(getChildFromState(state->applyCopy(action)));
	// children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return children.back();
}

sp<MCTSAgentBase::MCTSNode> MCTSAgentBase::MCTSNode::selectChild(param_t exploreSpeed) {
	assert(!children.empty());
	return *std::max_element(children.begin(), children.end(),
			[&exploreSpeed, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreSpeed) < UCT(ch2, exploreSpeed);
	});
}

param_t MCTSAgentBase::MCTSNode::UCT(const sp<MCTSNode>& v, param_t c) const {
	return param_t(v->stats.score) / v->stats.visits +
		c * std::sqrt(2 * std::log(stats.visits) / v->stats.visits);
}

up<State> MCTSAgentBase::MCTSNode::cloneState() {
	return state->clone();
}

void MCTSAgentBase::MCTSNode::addReward(reward_t delta) {
	stats.score += delta;
	++stats.visits;
}

sp<Action> MCTSAgentBase::MCTSNode::getBestAction() {
	int bestChildIdx = std::max_element(children.begin(), children.end(),
			[](const auto& ch1, const auto& ch2){
		return *ch1 < *ch2;
	}) - children.begin();
	assert(bestChildIdx < int(actions.size()));
	return actions[bestChildIdx];
}

bool MCTSAgentBase::MCTSNode::operator<(const MCTSNode& o) const {
	// return 1ll * stats.score * o.stats.visits < 1ll * o.stats.score * stats.score;
	return stats.visits < o.stats.visits;
}

void MCTSAgentBase::recordAction(const sp<Action>& action) {
	auto recordActionIdx = std::find_if(root->actions.begin(), root->actions.end(),
			[&action](const auto& x){
		return action->equals(x);
	}) - root->actions.begin();

	assert(recordActionIdx < int(root->actions.size()));
	if (recordActionIdx < int(root->children.size()))
		root = root->children[recordActionIdx];
	else
		root = root->getChildFromState(root->state->applyCopy(action));
		// root = std::mksh<MCTSNode>(root->state->applyCopy(action));
	assert(!root->parent.lock());
}

void MCTSAgentBase::changeCalcLimit(double newLimitInMs) {
	timer.changeLimit(newLimitInMs);
}

void MCTSAgentBase::postWork() {

}


class MCTSAgent : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;

  	MCTSAgent(AgentID id, const up<State> &initialState,
			double calcLimitInMs, const AgentArgs& args);

	std::vector<KeyValue> getDesc() const override;

protected:
	struct MCTSNode : public MCTSAgentBase::MCTSNode {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);
		sp<MCTSNodeBase> getChildFromState(up<State>&& state) override;
	};

	sp<MCTSAgentBase::MCTSNode> treePolicy() override;
	reward_t defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	void backup(sp<MCTSNodeBase> node, reward_t delta) override;
};


#include <cassert>

using param_t = MCTSAgent::param_t;
using reward_t = MCTSAgent::reward_t;
using MCTSNodeBase = MCTSAgent::MCTSNodeBase;

MCTSAgent::MCTSAgent(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args) {

}

MCTSAgent::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgent::treePolicy() {
	auto currentNode = root;
	descended = 0;

	while (!currentNode->isTerminal()) {
		++descended;
		if (currentNode->shouldExpand())
			return expand(currentNode);
		currentNode = currentNode->selectChild(exploreFactor);
	}

	return currentNode;
}

MCTSAgent::MCTSNode::MCTSNode(up<State>&& initialState)
	: MCTSNodeBase(std::move(initialState)) {

}

reward_t MCTSAgent::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();

     while (!state->isTerminal()) {
		auto actions = state->getValidActions();
		const auto& action = Random::choice(actions);
		state->apply(action);
	}

	return state->getReward(getID());
}

void MCTSAgent::backup(sp<MCTSNodeBase> node, reward_t delta) {
	int ascended = 0;

	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++ascended;
	}

	assert(descended + 1 == ascended);
}

sp<MCTSNodeBase> MCTSAgent::MCTSNode::getChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

std::vector<KeyValue> MCTSAgent::getDesc() const {
	auto averageSimulationCount = double(simulationCount) / timer.getTotalNumberOfCals();
	return { { "MCTS Agent with UCT selection and random simulation policy.", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) }
	};
}


class MCTSAgentWithMAST : public MCTSAgentBase {
public:
	using param_t = MCTSAgentBase::param_t;
	using reward_t = MCTSAgentBase::reward_t;
	using MCTSNodeBase = MCTSAgentBase::MCTSNode;
	using AgentArgs = Agent::AgentArgs;

	MCTSAgentWithMAST(AgentID id, const up<State> &initialState,
			double calcLimitInMs, const AgentArgs& args);
 
	std::vector<KeyValue> getDesc() const override;

private:
	struct MCTSNode : public MCTSNodeBase {
		MCTSNode(const up<State>& initialState);
		MCTSNode(up<State>&& initialState);

		sp<MCTSNodeBase> getChildFromState(up<State>&& state) override;
		int expandAndGetIdx();
		int selectAndGetIdx(param_t exploreFactor);
	};

	struct MASTActionStats {
		reward_t score = 0;
		int times = 0;
	};

	sp<MCTSNodeBase> treePolicy() override;
	int expandAndGetIdx(const sp<MCTSNode>& node);
	reward_t defaultPolicy(const sp<MCTSNodeBase>& initialNode) override;
	sp<Action> getActionWithDefaultPolicy(const up<State>& state);
	void backup(sp<MCTSNodeBase> node, reward_t delta) override;

	void postWork() override;
	void MASTPolicy(reward_t delta);
	inline void updateActionStat(AgentID id, int actionIdx, reward_t delta);

private:
	double epsilon;
	double decayFactor;
	int timesTreeDescended;
	int maxAgentCount;
	int maxActionCount;
	std::vector<std::vector<MASTActionStats>> actionsStats;
	std::vector<std::pair<AgentID, int>> actionHistory;
	int defaultPolicyLength;
};


#include <cassert>
#include <algorithm>

using param_t = MCTSAgentWithMAST::param_t;
using reward_t = MCTSAgentWithMAST::reward_t;
using MCTSNodeBase = MCTSAgentWithMAST::MCTSNodeBase;


MCTSAgentWithMAST::MCTSAgentWithMAST(AgentID id, const up<State>& initialState, 
		double calcLimitInMs, const AgentArgs& args) :
	MCTSAgentBase(id, std::mku<MCTSNode>(initialState), calcLimitInMs, args),
	epsilon(getOrDefault(args, "epsilon", 0.8)),
	decayFactor(getOrDefault(args, "decayFactor", 0.6)),
	maxAgentCount(initialState->getAgentCount()),
	maxActionCount(initialState->getActionCount()),
	actionsStats(maxAgentCount) {

	std::fill(actionsStats.begin(), actionsStats.end(),
			std::vector<MASTActionStats>(maxActionCount));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(const up<State>& initialState) :
	MCTSNodeBase(initialState) {

}

sp<MCTSNodeBase> MCTSAgentWithMAST::treePolicy() {
	auto currentNode = std::dynamic_pointer_cast<MCTSNode>(root);
	assert(currentNode);
	timesTreeDescended = 0;

	while (!currentNode->isTerminal()) {
		++timesTreeDescended;

		if (currentNode->shouldExpand()) {
			int expandIdx = expandAndGetIdx(currentNode);
			assert(expandIdx == int(currentNode->children.size() - 1));
			assert(expandIdx == currentNode->nextActionToResolveIdx - 1);

			actionHistory.emplace_back(currentNode->state->getTurn(), currentNode->actions[expandIdx]->getIdx());
			return currentNode->children[expandIdx];
		}
		
		int selectedChildIdx = currentNode->selectAndGetIdx(exploreFactor);
		assert(selectedChildIdx < int(currentNode->children.size()));
		assert(selectedChildIdx < int(currentNode->actions.size()));

		actionHistory.emplace_back(currentNode->state->getTurn(), currentNode->actions[selectedChildIdx]->getIdx());
		currentNode = std::dynamic_pointer_cast<MCTSNode>(currentNode->children[selectedChildIdx]);
		assert(currentNode);
	}

	return currentNode;
}

int MCTSAgentWithMAST::expandAndGetIdx(const sp<MCTSNode>& node) {
	int expandedIdx = node->expandAndGetIdx();
	node->children[expandedIdx]->parent = node;
	return expandedIdx;
}

int MCTSAgentWithMAST::MCTSNode::expandAndGetIdx() {
	assert(nextActionToResolveIdx == int(children.size()));
	assert(nextActionToResolveIdx < int(actions.size()));

	const auto& action = actions[nextActionToResolveIdx];
	children.push_back(std::mksh<MCTSNode>(state->applyCopy(action)));
	return nextActionToResolveIdx++;
}

sp<MCTSNodeBase> MCTSAgentWithMAST::MCTSNode::getChildFromState(up<State>&& state) {
	return std::mksh<MCTSNode>(std::move(state));
}

MCTSAgentWithMAST::MCTSNode::MCTSNode(up<State>&& initialState) :
	MCTSNodeBase(std::move(initialState)) {

}

int MCTSAgentWithMAST::MCTSNode::selectAndGetIdx(param_t exploreFactor) {
	assert(!children.empty());
	return std::max_element(children.begin(), children.end(),
			[&exploreFactor, this](const auto& ch1, const auto& ch2){
		return UCT(ch1, exploreFactor) < UCT(ch2, exploreFactor);
	}) - children.begin();
}

reward_t MCTSAgentWithMAST::defaultPolicy(const sp<MCTSNodeBase>& initialNode) {
	auto state = initialNode->cloneState();
	defaultPolicyLength = 0;

	while (!state->isTerminal()) {
		const auto action = getActionWithDefaultPolicy(state);
		actionHistory.emplace_back(state->getTurn(), action->getIdx());
		state->apply(action);
		++defaultPolicyLength;
	}

	return state->getReward(getID());
}

sp<Action> MCTSAgentWithMAST::getActionWithDefaultPolicy(const up<State>& state) {
	auto actions = state->getValidActions();
	// std::shuffle()
	assert(!actions.empty());

	if (Random::rand(1.0) <= epsilon)
		return Random::choice(actions);

	return *std::max_element(actions.begin(), actions.end(),
			[&state, this](const sp<Action>& a1, const sp<Action>& a2){
		const auto& s1 = actionsStats[state->getTurn()][a1->getIdx()];
		const auto& s2 = actionsStats[state->getTurn()][a2->getIdx()];
		return s1.score * s2.times < s2.score * s1.times;
	});
}

void MCTSAgentWithMAST::backup(sp<MCTSNodeBase> node, reward_t delta) {
	int timesTreeAscended = 0;

	while (node) {
		node->addReward(delta);
		node = node->parent.lock();
		++timesTreeAscended;
	}

	assert(timesTreeDescended + 1 == timesTreeAscended);
	MASTPolicy(delta);
}

void MCTSAgentWithMAST::MASTPolicy(reward_t delta) {
	assert(defaultPolicyLength + timesTreeDescended == int(actionHistory.size()));
	assert((delta == 1 && actionHistory.back().first == getID()) || 
		  (delta == 0 && actionHistory.back().first != getID()) ||
		   delta == 0.5);

	for (const auto& [agentID, actionIdx] : actionHistory)
		updateActionStat(agentID, actionIdx, delta);

	actionHistory.clear();
}

void MCTSAgentWithMAST::updateActionStat(AgentID id, int actionIdx, reward_t delta) {
	assert(id < int(actionsStats.size()));
	assert(actionIdx < int(actionsStats[id].size()));

	auto& statToUpdate = actionsStats[id][actionIdx];
	statToUpdate.score += delta;
	++statToUpdate.times;
}

void MCTSAgentWithMAST::postWork() {
	for (auto& v : actionsStats)
		for (auto& x : v)
			x.score *= decayFactor;
}

std::vector<KeyValue> MCTSAgentWithMAST::getDesc() const {
	auto averageSimulationCount = double(simulationCount) / timer.getTotalNumberOfCals();
	return { { "MCTS Agent with UCT selection and MAST policy with epsilon-greedy simulation.", "" },
		{ "Turn time limit", std::to_string(timer.getLimit()) + " ms" },
		{ "Average turn time", std::to_string(timer.getAverageCalcTime()) + " ms" },
		{ "Average number of simulations per turn", std::to_string(averageSimulationCount) },
		{ "Exploration speed constant (C) in UCT policy", std::to_string(exploreFactor) },
		{ "Epsilon constant (E) in MAST default policy", std::to_string(epsilon) },
		{ "Decay factor (gamma) in MAST global action table", std::to_string(decayFactor) }
	};
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

	typedef struct UltimateTicTacToeAction : public Action {
		UltimateTicTacToeAction(const AgentID& agentID, int row, int col,
				const TicTacToe::TicTacToeAction& action);

		bool equals(const sp<Action>& o) const override;
		int getIdx() const override;

		AgentID agentID;
		int row, col;
		TicTacToe::TicTacToeAction action;
	} action_t;

	bool isTerminal() const override;
	void apply(const sp<Action>& act) override;

	constexpr int getAgentCount() const override;
	constexpr int getActionCount() const override;

	std::vector<sp<Action>> getValidActions() override;
	bool isValid(const sp<Action>& act) const override;

	up<State> clone() override;
	bool didWin(AgentID id) const override; 
	reward_t getReward(AgentID id) const override;
	AgentID getTurn() const override;

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

constexpr int UltimateTicTacToe::getAgentCount() const {
	return 2;
}

constexpr int UltimateTicTacToe::getActionCount() const {
	return BOARD_SIZE * BOARD_SIZE * BOARD_SIZE * BOARD_SIZE;
}

int UltimateTicTacToe::UltimateTicTacToeAction::getIdx() const {
	int r = row * BOARD_SIZE + action.row;
	int c = col * BOARD_SIZE + action.col;
	return r * BOARD_SIZE * BOARD_SIZE + c;
}

AgentID UltimateTicTacToe::getTurn() const {
	assert(turn != NONE);
	return turn;
}


class TicTacToeRealAgent : public Agent {
public:
	TicTacToeRealAgent(AgentID id, const up<State>&, double, const AgentArgs&);
	sp<Action> getAction(const up<State>& state) override;
	std::vector<KeyValue> getDesc() const override;
	
private:
	bool isInRange(int idx) const;
};



#include <memory>
#include <cassert>

using UltimateTicTacToeAction = UltimateTicTacToe::UltimateTicTacToeAction;
using TicTacToeAction = TicTacToe::TicTacToeAction; 

TicTacToeRealAgent::TicTacToeRealAgent(AgentID id, const up<State>&, double, const AgentArgs&) : Agent(id) {

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
#include <map>

template<class game_t, class agent1_t, class agent2_t>
class GameRunner {
public:
	using AgentArgs = Agent::AgentArgs;

	GameRunner(double turnLimitInMs, const AgentArgs& agent1Args, const AgentArgs& agent2Args) :
		turnLimitInMs(turnLimitInMs), agent1Args(agent1Args), agent2Args(agent2Args) {
		
	}

	void playGames(int numberOfGames, bool verbose=false) {
		statSystem.reset();
		for (int i = 0; i < numberOfGames - 1; ++i)
			playGame(verbose);
		playGame(verbose, true);
		statSystem.showStats();
	}

	void playGame(bool verbose=false, bool lastGame=false) {
		announceGameStart();

		up<State> game = std::mku<game_t>();
		sp<Agent> agents[] {
			std::mksh<agent1_t>(AGENT1, game, turnLimitInMs, agent1Args),
			std::mksh<agent2_t>(AGENT2, game, turnLimitInMs, agent2Args)
		};

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

		if (lastGame)
			for (int i = 0; i < agentCount; ++i) {
				auto id = agents[i]->getID();
				statSystem.addDesc(std::to_string(id), agents[i]->getDesc());
			}

		announceGameEnd(game->getWinnerName());
	}

private:
	void announceGameStart() {
		statSystem.recordStart();
	}

	void announceGameEnd(const std::string& winner) {
		statSystem.recordEnd(winner);
	}

private:
	const int agentCount = 2;
	double turnLimitInMs;
	AgentArgs agent1Args;
	AgentArgs agent2Args;
	StatSystem statSystem;
};



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


template<class game_t, class agent_t>
class CGRunner {
public:
	using AgentArgs = Agent::AgentArgs;

	CGRunner(double turnLimitInMs, const AgentArgs& agentArgs) :
		turnLimitInMs(turnLimitInMs), agentArgs(agentArgs) {

	}

	void playGame() const {
		up<State> game = std::mku<game_t>();
		sp<Agent> agents[] {
			std::mksh<CGAgent>(AGENT1),
			std::mksh<agent_t>(AGENT2, game, firstTurnLimitInMs, agentArgs),
		};

		int agentCount = sizeof(agents) / sizeof(agents[0]);
		int turn = 0; 
		bool firstTurn = true;

		while (!game->isTerminal()) {
			auto& agent = agents[turn];
			sp<Action> action = agent->getAction(game);

			if (!action) {
				game = std::mku<game_t>();
				agents[0] = std::mksh<agent_t>(AGENT1, game, firstTurnLimitInMs, agentArgs);
				agents[1] = std::mksh<CGAgent>(AGENT2);
				continue;
			}
			
			if (!std::dynamic_pointer_cast<CGAgent>(agent)) {
				const auto& act = std::dynamic_pointer_cast<
					typename game_t::action_t>(action);
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

private:
	void printAction(const sp<typename game_t::action_t>& action) const {
		int gameRow = action->row, gameCol = action->col;
		int row = action->action.row, col = action->action.col;
		int posRow = gameRow * 3 + row;
		int posCol = gameCol * 3 + col;
		std::cout << posRow << " " << posCol << std::endl;
	}

private:
	double turnLimitInMs;
	AgentArgs agentArgs;
	double firstTurnLimitInMs = 1000;
};


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
	auto gameRunner = GameRunner<UltimateTicTacToe, MCTSAgentWithMAST, FlatMCTSAgent>(
		turnLimitInMs, {
			{ "exploreFactor", 0.4 },
			{ "epsilon", 0.8 },
			{ "decayFactor", 0.6 }
		},
		{ { "exploreFactor", 0.4 } }
	);
	gameRunner.playGames(numberOfGames);
#else
	auto cgRunner = CGRunner<UltimateTicTacToe, MCTSAgent>(
		turnLimitInMs, {
			{ "exploreFactor", 0.4 }
		}
	);
	cgRunner.playGame();
#endif

	return 0;
}
