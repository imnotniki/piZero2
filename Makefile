CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -ljansson

TARGET = pi-setup-tool
SRC = pi-setup-tool.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
