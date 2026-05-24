BUILD_ROOT ?= $(abspath ..)
BUILD_OUTPUT_DIR ?= $(BUILD_ROOT)/build
DEBUG ?= true

CC ?= gcc
AR ?= ar
RM ?= rm -f
MKDIR_P ?= mkdir -p

ifeq ($(DEBUG),true)
CFLAGS += -g -O0
VERSION = debug
else
CFLAGS += -O2
VERSION = release
endif

CFLAGS += -Wall -Wextra -std=c11 -I$(CURDIR)
LDLIBS += -lm

ifeq ($(OS),Windows_NT)
SOCKET_LIBS = -lws2_32
SHARED_SUPPORTED = false
else
SOCKET_LIBS =
SHARED_SUPPORTED = true
CFLAGS += -fPIC -D_POSIX_C_SOURCE=200809L
endif

LIB_NAME ?= m70_ezsocket
OBJ_DIR := $(BUILD_OUTPUT_DIR)/obj
LIB_DIR := $(BUILD_OUTPUT_DIR)/lib
BIN_DIR := $(BUILD_OUTPUT_DIR)/bin
TEST_DIR := $(BUILD_OUTPUT_DIR)/tests

LIB_SRCS := m70_error.c m70_ezsocket.c m70_giop.c m70_log.c socket.c utill.c
LIB_OBJS := $(addprefix $(OBJ_DIR)/,$(LIB_SRCS:.c=.o))

EXAMPLE_OBJ := $(OBJ_DIR)/main.o
EXAMPLE_BIN := $(BIN_DIR)/mitsubishi_cnc_m70_test

STATIC_LIB := $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB := $(LIB_DIR)/lib$(LIB_NAME).so

REGRESSION_TEST_SRC := $(BUILD_ROOT)/tests/socket_regression_test.c
REGRESSION_TEST_OBJ := $(OBJ_DIR)/socket_regression_test.o
REGRESSION_TEST_BIN := $(TEST_DIR)/socket_regression_test

TEST_LIBS := $(SOCKET_LIBS) $(LDLIBS)
ifeq ($(OS),Windows_NT)
TEST_LIBS +=
else
TEST_LIBS += -lpthread
endif

DEFAULT_TARGETS := $(STATIC_LIB) $(EXAMPLE_BIN)
ifeq ($(SHARED_SUPPORTED),true)
DEFAULT_TARGETS += $(SHARED_LIB)
endif

.PHONY: all static shared example test clean

all: $(DEFAULT_TARGETS)
	@echo "Built $(VERSION) artifacts in $(BUILD_OUTPUT_DIR)"

static: $(STATIC_LIB)

ifeq ($(SHARED_SUPPORTED),true)
shared: $(SHARED_LIB)
else
shared:
	@echo "Shared library target is only supported on POSIX/GCC toolchains."
endif

example: $(EXAMPLE_BIN)

test: $(REGRESSION_TEST_BIN)
	@echo "Running socket regression test..."
	@$(REGRESSION_TEST_BIN)

$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR) $(TEST_DIR):
	@$(MKDIR_P) $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(REGRESSION_TEST_OBJ): $(REGRESSION_TEST_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(CURDIR) -c $< -o $@

$(STATIC_LIB): $(LIB_OBJS) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(SHARED_LIB): $(LIB_OBJS) | $(LIB_DIR)
	$(CC) -shared -o $@ $^ $(SOCKET_LIBS) $(LDLIBS)

$(EXAMPLE_BIN): $(EXAMPLE_OBJ) $(STATIC_LIB) | $(BIN_DIR)
	$(CC) -o $@ $(EXAMPLE_OBJ) $(STATIC_LIB) $(SOCKET_LIBS) $(LDLIBS)

$(REGRESSION_TEST_BIN): $(REGRESSION_TEST_OBJ) $(STATIC_LIB) | $(TEST_DIR)
	$(CC) -o $@ $(REGRESSION_TEST_OBJ) $(STATIC_LIB) $(TEST_LIBS)

clean:
	rm -rf $(BUILD_OUTPUT_DIR)




