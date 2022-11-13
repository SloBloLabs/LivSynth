.PHONY: all build cmake clean format

BUILD_DIR := build
BUILD_TYPE ?= Debug
# BUILD_TYPE ?= Release

all: build

${BUILD_DIR}/Makefile:
	cmake \
		-B${BUILD_DIR} \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake: ${BUILD_DIR}/Makefile

build: cmake
	$(MAKE) -C ${BUILD_DIR} --no-print-directory -j

flash: all
	$(MAKE) -C ${BUILD_DIR} flash

SRCS := $(shell find . -name '*.[ch]' -or -name '*.[ch]pp')
format: $(addsuffix .format,${SRCS})

%.format: %
	clang-format -i $<

clean:
	rm -rf $(BUILD_DIR)
