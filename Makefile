CC = gcc
CFLAGS = -Wall -Wextra -pthread

TARGET = build/matriz
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf build

.PHONY: all clean
