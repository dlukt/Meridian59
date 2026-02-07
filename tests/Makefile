CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -I../include

TARGET = unit_tests
SOURCES = test_main.c ../util/crc.c ../util/md5.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
