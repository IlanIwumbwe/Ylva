CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O3
SRC_DIR = src
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/board.cpp $(SRC_DIR)/engine.cpp $(SRC_DIR)/evaluation.cpp $(SRC_DIR)/movegen.cpp $(SRC_DIR)/defs.cpp $(SRC_DIR)/zobrist.cpp $(SRC_DIR)/uci.cpp
OBJS = $(SRCS:.cpp=.o)
HDRS = $(SRCS:.cpp=.h)
TARGET = ylva

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(TARGET)
 
board.o: $(SRC_DIR)/defs.h $(SRC_DIR)/move.h $(SRC_DIR)/state.h
movegen.o : $(SRC_DIR)/board.h $(SRC_DIR)/move.h
engine.o : $(SRC_DIR)/move.h $(SRC_DIR)/board.h $(SRC_DIR)/movegen.h
zobrist.o : $(SRC_DIR)/defs.h
uci.o : $(SRC_DIR)/defs.h $(SRC_DIR)/engine.h
evaluation.o : $(SRC_DIR)/board.h

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)

