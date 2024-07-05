CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O3 -flto -MMD -MP -static -static-libgcc -march=native -mtune=native -mpopcnt -finline-functions -msse4.1 -msse4.2
LDFLAGS = -flto -static -static-libgcc
SRCS = $(wildcard src/*.cpp) 
OBJS = $(SRCS:.cpp=.o)
DEPENDENCIES = $(OBJS:.o=.d)
TARGET = ylva
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#-include $(DEPENDENCIES)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET) $(DEPENDENCIES)

