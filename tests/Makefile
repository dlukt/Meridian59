CXX ?= g++
CXXFLAGS ?= -std=c++20 -x c++ -Wall -Wextra -I../include -DBLAK_PLATFORM_LINUX -DUNIT_TEST

TARGET = unit_tests
SOURCES = test_main.cpp test_channel.cpp test_json_utils.cpp test_parse_logic.cpp test_term.cpp test_retrieve_value.cpp test_osd_linux.cpp ../util/crc.c ../util/md5.c ../util/rscload.c ../blakserv/time.c ../blakserv/json_utils.c ../blakserv/term.c ../blakserv/channel.c ../blakserv/osd_linux.c

TARGET_SENDMSG = sendmsg_tests
SOURCES_SENDMSG = test_sendmsg_optimization.cpp

all: $(TARGET) $(TARGET_SENDMSG)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

$(TARGET_SENDMSG): $(SOURCES_SENDMSG)
	$(CXX) $(CXXFLAGS) -o $(TARGET_SENDMSG) $(SOURCES_SENDMSG)

test: $(TARGET) $(TARGET_SENDMSG)
	./$(TARGET)
	./$(TARGET_SENDMSG)

clean:
	rm -f $(TARGET) $(TARGET_SENDMSG)

.PHONY: all test clean
