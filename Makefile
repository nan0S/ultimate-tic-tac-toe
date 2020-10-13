TARGET = main
EXENAME = tictactoe
OBJS = main.o \
	Common.o \
	State.o \
	Action.o \
	Player.o \
	RandomPlayer.o \
	TicTacToe.o \
	UltimateTicTacToe.o \
	GameRunner.o \
	StatSystem.o

CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
DFLAGS = -fsanitize=address -fsanitize=undefined

all: $(TARGET)

install: $(TARGET) clean

debug: CXXFLAGS += $(DFLAGS)
debug: $(TARGET)

nodebug: CXXFLAGS += -DNDEBUG
nodebug: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(EXENAME) $^

main.o: main.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp %.hpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf *.o
distclean: clean
	rm -f $(EXENAME)

.PHONY: clean
