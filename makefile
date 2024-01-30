CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRCS = src/main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: src/main.cpp src/helper_funcs.h src/defs.h src/move.h src/state.h src/movegen.h src/board.h src/run.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
