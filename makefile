SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

FLAGS = -Wall -Wextra -g -O3 -DDEV
TARGET = ylva

%.o : %.c
	gcc $(FLAGS) -o $@ -c $<

$(TARGET) : $(OBJS)
	gcc $(FLAGS) -o $@ $^

all: $(TARGET)

.PHONY: clean

clean:	
	rm -rf $(OBJS) $(TARGET)