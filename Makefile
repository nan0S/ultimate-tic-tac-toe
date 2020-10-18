TARGET = main
EXENAME = ultimate-tictactoe
OBJS = main.o \
	Common.o \
	State.o \
	Action.o \
	Agent.o \
	RandomAgent.o \
	TicTacToe.o \
	UltimateTicTacToe.o \
	GameRunner.o \
	StatSystem.o \
	FlatMCTSAgent.o \
	TicTacToeRealAgent.o \
	MCTSAgent.o \
	CGAgent.o \
	CGRunner.o \
	MCTSAgentWithMAST.o

CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wreorder -O3
DFLAGS = -fsanitize=address -fsanitize=undefined
RFLAGS = -Ofast -DNDEBUG

all: $(TARGET)

install: $(TARGET) clean

release: CXXFLAGS += $(RFLAGS)
release: $(TARGET)

debug: CXXFLAGS += $(DFLAGS)
debug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(EXENAME) $^

main.o: main.cpp
	$(CC) $(CXXFLAGS) -DLOCAL -c -o $@ $<

%.o: %.cpp %.hpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
distclean: clean
	rm -f $(EXENAME)

.PHONY: clean
