CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = tcp_server
SRC = src/main.c src/server.c
BIN = src/$(TARGET)

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

run: $(BIN)
	./$(BIN)

clean:
	rm -f src/$(TARGET) *.o

.PHONY: all run clean