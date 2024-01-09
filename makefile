CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = chess

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp helper_funcs.h defs.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)
