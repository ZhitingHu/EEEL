# comment it to speedup
DEBUG = 1

EEEL_DIR := $(shell readlink $(dir $(lastword $(MAKEFILE_LIST))) -f)

EEEL_SRC = $(wildcard $(EEEL_DIR)/src/*.cpp)
EEEL_HDR = $(wildcard $(EEEL_DIR)/src/*.hpp)
EEEL_BIN = $(EEEL_DIR)/bin
EEEL_OBJ = $(EEEL_SRC:.cpp=.o)
EEEL_THIRD_PARTY = ${EEEL_DIR}/third_party

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
           -std=c++0x
           #-std=c++11

INCFLAGS = -I${EEEL_THIRD_PARTY}/include
LDFLAGS = -L${EEEL_THIRD_PARTY}/lib \
          -lgflags \
          -lglog
          

# Debugging
ifeq ($(DEBUG), 1)
  COMMON_FLAGS += -DDEBUG
else
  COMMON_FLAGS += -DNDEBUG
endif

CXXFLAGS += $(COMMON_FLAGS)
LDFLAGS += $(COMMON_FLAGS)


all: $(EEEL_BIN)/eeel_main

$(EEEL_BIN):
	mkdir -p $(EEEL_BIN)

$(EEEL_BIN)/eeel_main: $(EEEL_OBJ) $(EEEL_BIN)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) \
	$(EEEL_OBJ) $(LDFLAGS) -o $@

$(EEEL_OBJ): %.o: %.cpp $(EEEL_HDR)
	$(CXX) $(CXXFLAGS) -Wno-unused-result \
		$(INCFLAGS) -c $< -o $@

clean:
	rm -rf $(EEEL_OBJ)
	rm -rf $(EEEL_BIN)

.PHONY: clean
