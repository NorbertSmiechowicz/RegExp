
CC					:= gcc
CXX					:= g++

SRC_ROOT 			:= src
SRC_DIRS			:= src
TARGET				:= build/app

CSTD				:= -std=c23
CXXSTD				:= -std=c++20

COMMON_FLAGS		:= -MMD -MP -Wall -Wextra -Wpedantic
DEBUG_FLAGS			:= -g -O0
RELEASE_FLAGS		:= -O3 -DNDEBUG

BUILD				?= debug

ifeq ($(BUILD),debug)
	CFLAGS			:= $(CSTD) $(COMMON_FLAGS) $(DEBUG_FLAGS)
    CXXFLAGS		:= $(CXXSTD) $(COMMON_FLAGS) $(DEBUG_FLAGS)
	BUILD_ROOT		:= build/debug
else
	CFLAGS			:= $(CSTD) $(COMMON_FLAGS) $(RELEASE_FLAGS)
    CXXFLAGS		:= $(CXXSTD) $(COMMON_FLAGS) $(RELEASE_FLAGS)
	BUILD_ROOT		:= build/release
endif

CSRC				:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
CXXSRC				:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

BUILD_SUBDIRS		:= $(patsubst $(SRC_ROOT)%,$(BUILD_ROOT)%,$(SRC_DIRS))

COBJS				:= $(patsubst $(SRC_ROOT)/%.c,$(BUILD_ROOT)/%.o,$(CSRC))
CXXOBJS				:= $(patsubst $(SRC_ROOT)/%.cpp,$(BUILD_ROOT)/%.o,$(CXXSRC))

OBJS				:= $(COBJS) $(CXXOBJS)

DEPENDANCY_FILES := $(OBJS:.o=.d)
-include $(DEPENDANCY_FILES)

LOG_MSG		= @echo "$(1)" && echo "[$$( date '+%FT%H:%M:%S.%6N%:z' )]"

$(TARGET): $(OBJS)
	$(call LOG_MSG,linking: $@)
	$(CXX) $(OBJS) -o $@

$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

$(COBJS): $(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.c | $(BUILD_SUBDIRS)
	$(call LOG_MSG,compiling: $@)
	$(CC) $(CFLAGS) -c $^ -o $@

$(CXXOBJS): $(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.cpp | $(BUILD_SUBDIRS)
	$(call LOG_MSG,compiling: $@)
	$(CXX) $(CXXFLAGS) -c $^ -o $@

debug:
	$(MAKE) -k BUILD=debug

release:
	$(MAKE) -k BUILD=release

clean:
	$(call LOG_MSG,$(CSRC))
	$(call LOG_MSG,$(BUILD_SUBDIRS))
	rm -rf build

.PHONY: debug release clean
