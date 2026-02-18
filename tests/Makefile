CXX ?= g++
CXXFLAGS ?= -std=c++20 -x c++ -Wall -Wextra -I../include -DBLAK_PLATFORM_LINUX -DUNIT_TEST

TARGET = unit_tests
SOURCES = test_main.cpp test_channel.cpp test_json_utils.cpp test_parse_logic.cpp test_term.cpp test_retrieve_value.cpp test_osd_linux.cpp test_webhook_message.cpp ../util/crc.c ../util/md5.c ../util/rscload.c ../blakserv/time.c ../blakserv/json_utils.c ../blakserv/term.c ../blakserv/channel.c ../blakserv/osd_linux.c ../blakserv/webhook_message.c

TARGET_SENDMSG = sendmsg_tests
SOURCES_SENDMSG = test_sendmsg_optimization.cpp

TARGET_INTERP = interpreter_tests
SOURCES_INTERP = test_interpreter.cpp

all: $(TARGET) $(TARGET_SENDMSG) $(TARGET_INTERP)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

$(TARGET_SENDMSG): $(SOURCES_SENDMSG)
	$(CXX) $(CXXFLAGS) -o $(TARGET_SENDMSG) $(SOURCES_SENDMSG)

$(TARGET_INTERP): $(SOURCES_INTERP)
	$(CXX) $(CXXFLAGS) -o $(TARGET_INTERP) $(SOURCES_INTERP)

test: $(TARGET) $(TARGET_SENDMSG) $(TARGET_INTERP)
	./$(TARGET)
	./$(TARGET_SENDMSG)
	./$(TARGET_INTERP)

clean:
	rm -f $(TARGET) $(TARGET_SENDMSG) $(TARGET_INTERP)

.PHONY: all test clean
