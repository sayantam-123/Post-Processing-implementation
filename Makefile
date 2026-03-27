CC = g++
LIBS = -lraylib
TARGET = post_processing
SRC = post_processing.cpp

all: build run clean

build:
	$(CC) $(SRC) -o $(TARGET) $(LIBS)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: build run clean
