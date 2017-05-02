CC = javac
SOURCE = $(wildcard *.java)
TARGET = Reversi.jar

default: $(TARGET)

Reversi.jar:
	$(CC) $(SOURCE)
	jar cfe $@ Reversi *.class *.jpg
	mv $(TARGET) ../.

clean:
	rm -f *.class ../$(TARGET)
