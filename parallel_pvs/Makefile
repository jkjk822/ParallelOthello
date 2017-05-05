CC = g++
CFLAGS = -O3 -pthread -std=c++11
DEBUG = -g
DEPS = structs.h ctpl.h
SOURCE = $(wildcard *.cpp)
OBJECTS = $(SOURCE:.cpp=.o)
TARGET = play

default: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

debug:
	$(CC) $(DEBUG) $(CFLAGS) $(SOURCE) -o $(TARGET)

play: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)