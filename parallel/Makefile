CC = g++
CFLAGS = -O3 -std=c++11
DEPS = structs.h
SOURCE = $(wildcard *.cpp)
OBJECTS = $(SOURCE:.cpp=.o)
TARGET = play

default: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

play: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)