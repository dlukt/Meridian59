CXX ?= g++
CXXFLAGS ?= -std=c++20 -x c++ -Wall -Wextra -I../include -DBLAK_PLATFORM_LINUX

TARGET = unit_tests
SOURCES = test_main.cpp ../util/crc.c ../util/md5.c ../blakserv/time.c

TARGET_CLASS_PTR = test_class_ptr_optimization
SOURCES_CLASS_PTR = test_class_ptr_optimization.cpp

all: $(TARGET) $(TARGET_CLASS_PTR)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

$(TARGET_CLASS_PTR): $(SOURCES_CLASS_PTR)
	$(CXX) $(CXXFLAGS) -o $(TARGET_CLASS_PTR) $(SOURCES_CLASS_PTR)

test: $(TARGET) $(TARGET_CLASS_PTR)
	./$(TARGET)
	./$(TARGET_CLASS_PTR)

clean:
	rm -f $(TARGET) $(TARGET_CLASS_PTR)

.PHONY: all test clean
