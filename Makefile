
PROJECT := eeel

# comment it to avoid omp
OPENMP = 0
# comment it to speedup
#DEBUG = 1
# comment it if use double
USEFLOAT = 1

EEEL_DIR := $(shell readlink $(dir $(lastword $(MAKEFILE_LIST))) -f)
BUILD_DIR = $(EEEL_DIR)/build

##############################
# EEEL Library
##############################
EEEL_SRCS = $(shell find src -name "*.cpp")
EEEL_HDR = $(shell find src -name "*.hpp")
EEEL_OBJS = $(addprefix $(BUILD_DIR)/, ${EEEL_SRCS:.cpp=.o})
EEEL_OBJ_BUILD_DIR = $(BUILD_DIR)/src
EEEL_LIB_BUILD_DIR = $(BUILD_DIR)/lib
EEEL_THIRD_PARTY = ${EEEL_DIR}/third_party

EEEL_STATIC_NAME = $(EEEL_LIB_BUILD_DIR)/lib$(PROJECT).a

##############################
# EEEL tools
##############################
TOOL_SRCS := $(shell find tools -name "*.cpp")
TOOL_OBJS := $(addprefix $(BUILD_DIR)/, ${TOOL_SRCS:.cpp=.o})
TOOL_BUILD_DIR := $(BUILD_DIR)/tools
TOOL_BINS := ${TOOL_OBJS:.o=.bin}
TOOL_BIN_LINKS := ${TOOL_BINS:.bin=}

##############################
# Flags
##############################
CXX = g++
CXXFLAGS = -g \
           -O3 \
           -Wall \
           -Wno-sign-compare \
           -fno-builtin-malloc \
           -fno-builtin-calloc \
           -fno-builtin-realloc \
           -fno-builtin-free \
           -fno-omit-frame-pointer \
           -std=c++11
           #-std=c++0x -pthread

INCFLAGS = -I${EEEL_THIRD_PARTY}/include -I./src
LDFLAGS = -L${EEEL_THIRD_PARTY}/lib \
          -lgflags \
          -lglog


ifeq ($(OPENMP), 1)
  COMMON_FLAGS += -DOPENMP
  CXXFLAGS += -fopenmp
else
  COMMON_FLAGS += -DNOPENMP
endif

# Debugging
ifeq ($(DEBUG), 1)
  COMMON_FLAGS += -DDEBUG
else
  COMMON_FLAGS += -DNDEBUG
endif

ifeq ($(USEFLOAT), 1)
  COMMON_FLAGS += -DUSEFLOAT
else
  COMMON_FLAGS += -DUSEDOUBLE
endif

CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += $(COMMON_FLAGS)



##############################
# Set build directories
##############################

ALL_BUILD_DIRS := $(sort \
		$(BUILD_DIR) $(EEEL_LIB_BUILD_DIR) $(EEEL_OBJ_BUILD_DIR) \
                $(TOOL_BUILD_DIR))

all: $(EEEL_STATIC_NAME) tools

$(ALL_BUILD_DIRS):
	@ mkdir -p $@

$(EEEL_STATIC_NAME): $(EEEL_OBJS) | $(EEEL_LIB_BUILD_DIR)
	ar rcs $@ $(EEEL_OBJS)
	@ echo

#$(EEEL_OBJS): %.o: %.cpp $(EEEL_HDR) | $(EEEL_OBJ_BUILD_DIR)
#	$(CXX) $(CXXFLAGS) -Wno-unused-result \
#	       $(INCFLAGS) -c $< -o $@
#$(EEEL_OBJS): %.o: $(EEEL_SRC) $(EEEL_HDR) | $(EEEL_OBJ_BUILD_DIR)
#	$(CXX) $(CXXFLAGS) -Wno-unused-result \
#	       $(INCFLAGS) -c $< -o $@
$(EEEL_OBJ_BUILD_DIR)/%.o: src/%.cpp $(EEEL_HDR) | $(EEEL_OBJ_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Wno-unused-result \
	       $(INCFLAGS) -c $< -o $@
	@ echo

tools: $(TOOL_BINS) $(TOOL_BIN_LINKS)

# Target for extension-less symlinks to tool binaries with extension '*.bin'.
$(TOOL_BUILD_DIR)/%: $(TOOL_BUILD_DIR)/%.bin | $(TOOL_BUILD_DIR)
	@ $(RM) $@
	@ ln -s $(abspath $<) $@

$(TOOL_BINS): %.bin : %.o $(EEEL_STATIC_NAME)
	$(CXX) $< $(EEEL_STATIC_NAME) $(CXXFLAGS) $(INCFLAGS) $(LINKFLAGS) \
        $(LDFLAGS) -o $@
	@ echo

#$(TOOL_OBJS): 
$(TOOL_BUILD_DIR)/%.o: tools/%.cpp $(EEEL_HDR) | $(TOOL_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Wno-unused-result \
               $(INCFLAGS) -c $< -o $@
	@ echo

#$(BUILD_DIR)/eeel_main: $(EEEL_OBJ) $(EEEL_BUILD)
#	$(CXX) $(CXXFLAGS) $(INCFLAGS) \
#	$(EEEL_OBJS) $(LDFLAGS) -o $@


clean:
	@- $(RM) -rf $(ALL_BUILD_DIRS)

.PHONY: clean
