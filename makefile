
CC					:= gcc
CXX				:= g++

SRC_ROOT 			:= src
SRC_DIRS			:= src
TARGET				:= build/app

CSTD				:= -std=c23
CXXSTD				:= -std=c++20
LDFLAGS			:=

# SAN_FLAGS			:= -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer
SAN_FLAGS			:=
COMMON_FLAGS		:= -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnrvo -Wswitch-enum -Werror
DEBUG_FLAGS		:= -g -O0
RELEASE_FLAGS		:= -O3 -DNDEBUG

BUILD				?= debug

ifeq ($(BUILD),debug)
	CFLAGS			:= $(CSTD) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(SAN_FLAGS)
    CXXFLAGS		:= $(CXXSTD) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(SAN_FLAGS)
	LDFLAGS		:= $(LDFLAGS) $(SAN_FLAGS)
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
CXXOBJS			:= $(patsubst $(SRC_ROOT)/%.cpp,$(BUILD_ROOT)/%.o,$(CXXSRC))

OBJS				:= $(COBJS) $(CXXOBJS)

DEPENDANCY_FILES := $(OBJS:.o=.d)
-include $(DEPENDANCY_FILES)

# LOG_MSG		= @echo "[$$( date '+%FT%H:%M:%S.%6N%:z' )] $(1)"

debug:
	$(MAKE) -k -j8 BUILD=debug build/app

release:
	$(MAKE) -k -j8 BUILD=release build/app

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_SUBDIRS):
	mkdir -p $(BUILD_SUBDIRS)

$(COBJS): $(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.c | $(BUILD_SUBDIRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(CXXOBJS): $(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.cpp | $(BUILD_SUBDIRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: debug release clean
