CC = gcc
CFLAGS = -O3 -std=c99 
DEPS = structs.h
SOURCE = $(wildcard *.c)
OBJECTS = $(SOURCE:.c=.o)
TARGET = play

default: $(TARGET)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

play: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)