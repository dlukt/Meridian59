CXX ?= g++
CXXFLAGS ?= -std=c++20 -x c++ -Wall -Wextra -I../include -DBLAK_PLATFORM_LINUX -DUNIT_TEST

TARGET = unit_tests
SOURCES = test_main.cpp test_channel.cpp test_json_utils.cpp test_parse_logic.cpp test_term.cpp test_retrieve_value.cpp test_osd_linux.cpp test_webhook_message.cpp ../util/crc.c ../util/md5.c ../util/rscload.c ../blakserv/time.c ../blakserv/json_utils.c ../blakserv/term.c ../blakserv/channel.c ../blakserv/osd_linux.c ../blakserv/webhook_message.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
