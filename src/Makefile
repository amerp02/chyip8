CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lm -lSDL2 -lSDL2_image
SRC = main.c chip8.c
OBJ = $(SRC:.c=.o)
TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
