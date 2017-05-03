CC = gcc
CFLAGS = -O3 -std=c99 
DEBUG = -g
DEPS = structs.h
SOURCE = $(wildcard *.c)
OBJECTS = $(SOURCE:.c=.o)
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
