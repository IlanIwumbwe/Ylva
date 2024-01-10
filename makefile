CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
SRCS = src/main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: src/main.cpp src/helper_funcs.h src/defs.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
