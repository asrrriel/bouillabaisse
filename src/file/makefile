CXX = clang++

CXXFLAGS := -g -Wall -Wextra -std=c++20 -I../../${BUILD_DIR}/${LIB_DIR}/include -DVERSION='"${VERSION}"' -I ../include
LDFLAGS  := -g --fuse-ld=lld

FILES := $(shell find . -name '*.cpp')

OBJS := $(FILES:.cpp=.o)
OBJS := $(patsubst .%,../../${BUILD_DIR}/${OBJ_DIR}%,$(OBJS))

all: $(OBJS)
	ar rc ../../${BUILD_DIR}/${BIN_DIR}/file.a $(OBJS)

../../${BUILD_DIR}/${OBJ_DIR}/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@