# Makefile for Tiny Pointer Library
# Directory structure:
#   src      - Source files (e.g. tiny_ptr.c)
#   include - Header files (e.g. tiny_ptr.h)
#   tests    - Test files (e.g. test_tiny_ptr.cpp)
#   build    - Build artifacts (object files, static library, test executable)
#
# This Makefile builds the static library by default.
# To compile the tests (which use Google Test), run "make tests".

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -Iinclude -pthread
CXXFLAGS = -Wall -O2 -Iinclude -pthread
AR = ar rcs

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Output object files
SIMPLE_OBJS = $(BUILD_DIR)/tiny_ptr_simple.o
FIXED_OBJS = $(BUILD_DIR)/tiny_ptr_fixed.o
VARIABLE_OBJS = $(BUILD_DIR)/tiny_ptr_variable.o
UNIFIED_OBJS = $(BUILD_DIR)/tiny_ptr_unified.o

# Library targets
LIB_SIMPLE = $(BUILD_DIR)/libtiny_ptr_simple.a
LIB_FIXED = $(BUILD_DIR)/libtiny_ptr_fixed.a
LIB_VARIABLE = $(BUILD_DIR)/libtiny_ptr_variable.a
LIB_UNIFIED = $(BUILD_DIR)/libtiny_ptr_unified.a

# Test executables
TEST_SIMPLE = $(BUILD_DIR)/test_simple
TEST_FIXED = $(BUILD_DIR)/test_fixed
TEST_VARIABLE = $(BUILD_DIR)/test_variable

# Google Test integration as a third–party library
GTEST_DIR = $(TEST_DIR)/googletest/googletest
GTEST_SRC = $(GTEST_DIR)/src/gtest-all.cc
GTEST_OBJS = $(BUILD_DIR)/gtest-all.o
LIB_GTEST = $(BUILD_DIR)/libgtest.a

.PHONY: all simple fixed variable clean tests build_gtest build_unified test_simple test_fixed test_variable

all: $(LIB_SIMPLE) $(LIB_FIXED) $(LIB_VARIABLE) $(LIB_UNIFIED)

simple: $(LIB_SIMPLE)

fixed: $(LIB_FIXED)

variable: $(LIB_VARIABLE)

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build object files
$(BUILD_DIR)/gtest-all.o: $(GTEST_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include -I$(GTEST_DIR) -c $< -o $@

$(BUILD_DIR)/tiny_ptr_simple.o: $(SRC_DIR)/tiny_ptr_simple.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tiny_ptr_fixed.o: $(SRC_DIR)/tiny_ptr_fixed.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tiny_ptr_variable.o: $(SRC_DIR)/tiny_ptr_variable.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tiny_ptr_unified.o: $(SRC_DIR)/tiny_ptr_unified.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build libraries
$(LIB_GTEST): $(BUILD_DIR)/gtest-all.o
	$(AR) $@ $^

$(LIB_SIMPLE): $(BUILD_DIR)/tiny_ptr_simple.o
	$(AR) $@ $^

$(LIB_FIXED): $(BUILD_DIR)/tiny_ptr_simple.o $(BUILD_DIR)/tiny_ptr_fixed.o
	$(AR) $@ $^

$(LIB_VARIABLE): $(BUILD_DIR)/tiny_ptr_simple.o $(BUILD_DIR)/tiny_ptr_variable.o
	$(AR) $@ $^

$(LIB_UNIFIED): $(BUILD_DIR)/tiny_ptr_simple.o $(BUILD_DIR)/tiny_ptr_fixed.o $(BUILD_DIR)/tiny_ptr_variable.o $(BUILD_DIR)/tiny_ptr_unified.o
	$(AR) $@ $^

# Test targets
build_gtest: $(LIB_GTEST)

build_unified: $(LIB_UNIFIED)

test_simple: $(LIB_SIMPLE)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include $(TEST_DIR)/test_tiny_ptr_simple.cpp -L$(BUILD_DIR) $(LIB_UNIFIED) $(LIB_GTEST) -lpthread -o $(TEST_SIMPLE)
	./$(TEST_SIMPLE)

test_fixed: $(LIB_FIXED)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include $(TEST_DIR)/test_tiny_ptr_fixed.cpp -L$(BUILD_DIR) $(LIB_UNIFIED) $(LIB_GTEST) -lpthread -o $(TEST_FIXED)
	./$(TEST_FIXED)

test_variable: $(LIB_VARIABLE)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include $(TEST_DIR)/test_tiny_ptr_variable.cpp -L$(BUILD_DIR) $(LIB_UNIFIED) $(LIB_GTEST) -lpthread -o $(TEST_VARIABLE)
	./$(TEST_VARIABLE)

tests: build_gtest build_unified test_simple test_fixed test_variable

clean:
	rm -rf $(BUILD_DIR)
