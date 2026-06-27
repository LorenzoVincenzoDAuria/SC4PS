CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
TARGET  = daxpy
SRC     = daxpy.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

# Example run with the values requested in the assignment (a=3, x=0.1, y=7.1)
run: $(TARGET)
	./$(TARGET) 10 3 0.1 7.1

clean:
	rm -f $(TARGET)
