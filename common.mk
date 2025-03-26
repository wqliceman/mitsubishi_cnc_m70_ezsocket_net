
#.PHONY:all clean 

ifeq ($(DEBUG),true)
#-g generates debugging information. GNU debugger can use this information
CC = gcc -g
VERSION = debug
else
CC = gcc
VERSION = release
endif

#CC = gcc

# $(wildcard *.c) scans all .c files in the current directory
#SRCS = nginx.c ngx_conf.c
SRCS = $(wildcard *.c)

OBJS = $(SRCS:.c=.o)

#Replace .c with .d in the string
#DEPS = nginx.d ngx_conf.d
DEPS = $(SRCS:.c=.d)

#Can specify the location of BIN file, addprefix is a function to add prefix
#BIN = /mnt/hgfs/linux/nginx
BIN := $(addprefix $(BUILD_ROOT)/,$(BIN))

#Define the directory for storing obj files, centralizing them in one location makes linking easier, otherwise they would be scattered in subdirectories making linking difficult
#Note that the string below should not have spaces at the end, otherwise it will cause syntax errors 
LINK_OBJ_DIR = $(BUILD_ROOT)/app/link_obj
DEP_DIR = $(BUILD_ROOT)/app/dep

#-p recursively creates directories, creates if they don't exist, no action if they already exist
$(shell mkdir -p $(LINK_OBJ_DIR))
$(shell mkdir -p $(DEP_DIR))

OBJS := $(addprefix $(LINK_OBJ_DIR)/,$(OBJS))
DEPS := $(addprefix $(DEP_DIR)/,$(DEPS))

#Find all .o files in the directory (compiled files)
LINK_OBJ = $(wildcard $(LINK_OBJ_DIR)/*.o)
#Because this .o file in the app directory hasn't been built yet when building dependencies, LINK_OBJ is missing this .o file, so we need to add it
LINK_OBJ += $(OBJS)

#-------------------------------------------------------------------------------------------------------
all:$(DEPS) $(OBJS) $(BIN)

ifneq ("$(wildcard $(DEPS))","")   #If not empty, $(wildcard) is a function [get matching pattern filenames], used here to compare if it's ""
include $(DEPS)  
endif

#----------------------------------------------------------------1begin------------------
#$(BIN):$(OBJS)
$(BIN):$(LINK_OBJ)
	@echo "------------------------build $(VERSION) mode--------------------------------!!!"

ifeq ($(BUILD_SO), true)
# gcc -o generates a shared object (.so) file
	$(CC) -fPIC -shared -o $@.so $^
else
# gcc -o generates an executable file
	$(CC) -o $@ $^
endif

#----------------------------------------------------------------1end-------------------


#----------------------------------------------------------------2begin-----------------
#%.o:%.c
$(LINK_OBJ_DIR)/%.o:%.c
#$(CC) -o $@ -c $^
	$(CC) -I$(INCLUDE_PATH) -o $@ -c $(filter %.c,$^)
#----------------------------------------------------------------2end-------------------



#----------------------------------------------------------------3begin-----------------
$(DEP_DIR)/%.d:%.c
#gcc -MM $^ > $@
	echo -n $(LINK_OBJ_DIR)/ > $@
#	gcc -MM $^ | sed 's/^/$(LINK_OBJ_DIR)&/g' > $@
#  >> means append
	gcc -I$(INCLUDE_PATH) -MM $^ >> $@
#----------------------------------------------------------------3begin-----------------



#----------------------------------------------------------------nbegin-----------------
clean:			
	rm -f $(BIN) $(OBJS) $(DEPS) *.gch
#----------------------------------------------------------------nend------------------





