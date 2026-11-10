CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Iinclude
TARGET = casefile

SOURCES = \
	src/main.c \
	src/game.c \
	src/map.c \
	src/investigation.c \
	src/evidence.c \
	src/courtroom.c \
	src/files.c \
	src/locker.c \
	src/dialogue.c \
	src/suspects.c

.PHONY: all run clean test test-build

all: $(TARGET)

$(TARGET): $(SOURCES) $(wildcard include/*.h)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Build the separate test executables. Each test has its own main().
test-build: test_person1 test_files test_courtroom test_locker

# Run the automated save/load test suite.
test: test_files test_locker
	./test_files
	./test_locker

test_person1: tests/test_person1.c src/suspects.c src/dialogue.c
	$(CC) $(CFLAGS) tests/test_person1.c src/suspects.c src/dialogue.c -o $@

test_files: tests/test_files.c src/files.c src/evidence.c src/locker.c
	$(CC) $(CFLAGS) tests/test_files.c src/files.c src/evidence.c src/locker.c -o $@

test_courtroom: tests/test_courtroom.c src/courtroom.c
	$(CC) $(CFLAGS) tests/test_courtroom.c src/courtroom.c -o $@

test_locker: tests/test_locker.c src/locker.c
	$(CC) $(CFLAGS) tests/test_locker.c src/locker.c -o $@

# IMPORTANT: clean removes generated binaries and temporary test saves only.
# It deliberately preserves savegame.dat, the player's actual saved game.
clean:
	rm -f $(TARGET) test_person1 test_files test_courtroom test_locker \
		test_save.dat test_empty.dat test_corrupt.dat test_locker_input.txt file_that_does_not_exist.dat
