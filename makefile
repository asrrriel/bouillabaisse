CXX := clang++
LD := clang++

VERSION := $(shell git rev-parse --short HEAD)

DEBUG_CXXFLAGS = -g -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable -fsanitize=address -fsanitize=undefined 
DEBUG_LDFLAGS  = -g -fsanitize=address -fsanitize=undefined

CXXFLAGS := -std=c++20 -I../../${BUILD_DIR}/${LIB_DIR}/include -DVERSION='"${VERSION}"' -I ../include $(DEBUG_CXXFLAGS)
LDFLAGS  := -fuse-ld=lld -lfmt -lasound $(DEBUG_LDFLAGS)

BUILD_DIR := build
OBJ_DIR := obj
BIN_DIR := bin
LIB_DIR := lib

export CXX CXXFLAGS VERSION BUILD_DIR OBJ_DIR BIN_DIR LIB_DIR

COMPONENTS = app file io aumidi

.PHONY: all
all: $(COMPONENTS)
	$(LD) $(LDFLAGS) -o ${BUILD_DIR}/bouillabaisse $(patsubst %,${BUILD_DIR}/${BIN_DIR}/%.a,$(COMPONENTS))

$(COMPONENTS): dirs libs
	$(MAKE) -C src/$@

dirs:
	@mkdir -p ${BUILD_DIR}
	@mkdir -p ${BUILD_DIR}/${OBJ_DIR}
	@mkdir -p ${BUILD_DIR}/${BIN_DIR}

	@mkdir -p ${LIB_DIR}
	@mkdir -p ${BUILD_DIR}/${LIB_DIR}

.PHONY: libs
libs: dirs
	@mkdir -p ${BUILD_DIR}/${LIB_DIR}/include
ifeq ($(wildcard ${BUILD_DIR}/${LIB_DIR}/include/spdlog),)
	wget https://github.com/gabime/spdlog/archive/refs/tags/v1.15.2.zip -O ${LIB_DIR}/spdlog.zip
	unzip ${LIB_DIR}/spdlog.zip -d ${LIB_DIR} 

	cp -r ${LIB_DIR}/spdlog-1.15.2/include/spdlog ${BUILD_DIR}/${LIB_DIR}/include

	rm -rf ${LIB_DIR}/spdlog-1.15.2
endif

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR}

.PHONY: bear
bear:
	bear          -- $(MAKE) -B -C src/app
	bear --append -- $(MAKE) -B -C src/file
	bear --append -- $(MAKE) -B -C src/io
	bear --append -- $(MAKE) -B -C src/aumidi

format:
	clang-format -i $(shell find src -name '*.cpp') $(shell find src -name '*.h') $(shell find src -name '*.hpp')

.PHONY: run
run: all
	./${BUILD_DIR}/bouillabaisse