
include config.mk

.PHONY: all static shared example test clean

all:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir all; \
	done

static:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir static; \
	done

shared:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir shared; \
	done

example:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir example; \
	done

test:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir test; \
	done

clean:
	@for dir in $(BUILD_DIR); \
	do \
		$(MAKE) -C $$dir clean; \
	done
	rm -rf $(BUILD_OUTPUT_DIR) app/link_obj app/dep nginx
	rm -rf signal/*.gch app/*.gch

