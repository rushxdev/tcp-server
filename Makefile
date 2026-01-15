CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = tcp_server
SRC = src/main.c src/server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o src/$(TARGET)

clean:
	rm -f src/$(TARGET) *.o