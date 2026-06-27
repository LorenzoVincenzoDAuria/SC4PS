CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
TARGET  = matmul
SRC     = matmul.c

.PHONY: all clean run benchmark

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

run: $(TARGET)
	./$(TARGET) 2 3 100 C_out.txt ijk

# Benchmarks all 6 loop orderings at a fixed N (override with: make benchmark N=800)
N ?= 600
benchmark: $(TARGET)
	@for order in ijk ikj jik jki kij kji; do \
		./$(TARGET) 2 3 $(N) /tmp/C_$$order.txt $$order ; \
	done

clean:
	rm -f $(TARGET) C_out.txt /tmp/C_*.txt
