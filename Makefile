CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Isrc
SRCS   = src/main.c src/models.c src/dataset.c src/metrics.c src/oracle.c \
         src/simulator.c src/strategies.c src/kill.c src/retry.c src/rollback.c \
         src/benchmark.c
TARGET = deadlock_sim

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

run: $(TARGET)
	./$(TARGET)
	python3 src/visualize.py

clean:
	rm -f $(TARGET)
	rm -f results/*.csv
	rm -f figures/*.png

