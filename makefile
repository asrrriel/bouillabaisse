LD := clang++
LDFLAGS  := -g -fuse-ld=lld -lfmt -lasound

BUILD_DIR := build
OBJ_DIR := obj
BIN_DIR := bin
LIB_DIR := lib

VERSION := $(shell git rev-parse --short HEAD)

export VERSION BUILD_DIR OBJ_DIR BIN_DIR LIB_DIR

.PHONY: all
all: libs
	$(MAKE) -C src/app
	$(MAKE) -C src/io
#TODO: the rest of the app 
	$(LD) $(LDFLAGS) -o ${BUILD_DIR}/bouillabaisse $(shell find ${BUILD_DIR}/${BIN_DIR} -name '*.a')

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


.PHONY: run
run: all
	./${BUILD_DIR}/bouillabaisse