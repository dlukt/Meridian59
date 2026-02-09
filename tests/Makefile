CXX ?= g++
CXXFLAGS ?= -std=c++20 -x c++ -Wall -Wextra -I../include -DBLAK_PLATFORM_LINUX

TARGET = unit_tests
SOURCES = test_main.cpp ../util/crc.c ../util/md5.c ../util/rscload.c ../blakserv/time.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
