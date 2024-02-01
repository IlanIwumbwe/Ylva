CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O1 -O2 -O3
SRCS = src/main.cpp 
OBJS = $(SRCS:.cpp=.o)
HDRS = src/evaluation.h src/helper_funcs.h src/defs.h src/move.h src/state.h src/movegen.h src/board.h src/run.h src/engine.h
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)

