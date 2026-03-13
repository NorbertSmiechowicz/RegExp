# Compiler
CXX := g++
STD := -std=c++20

# Directories
SRC_DIR := src
BUILD_DIR := build
TARGET := app

# Build type (default = release)
BUILD ?= debug

# Flags
COMMON_FLAGS := $(STD) -Wall -Wextra -Wpedantic
DEBUG_FLAGS := -g -O0
RELEASE_FLAGS := -O3 -DNDEBUG

ifeq ($(BUILD),debug)
    CXXFLAGS := $(COMMON_FLAGS) $(DEBUG_FLAGS)
    BUILD_SUBDIR := $(BUILD_DIR)/debug
else
    CXXFLAGS := $(COMMON_FLAGS) $(RELEASE_FLAGS)
    BUILD_SUBDIR := $(BUILD_DIR)/release
endif

# Sources
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Objects
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_SUBDIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

# Compile
$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_SUBDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Build directory
$(BUILD_SUBDIR):
	mkdir -p $(BUILD_SUBDIR)

# Run
run: $(TARGET)
	./$(TARGET)

# Debug shortcut
debug:
	$(MAKE) BUILD=debug

# Release shortcut
release:
	$(MAKE) BUILD=release

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Include dependencies
-include $(DEPS)

.PHONY: all clean run debug release
