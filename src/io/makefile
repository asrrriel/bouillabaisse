FILES := $(shell find . -name '*.cpp')

OBJS := $(FILES:.cpp=.o)
OBJS := $(patsubst .%,../../${BUILD_DIR}/${OBJ_DIR}%,$(OBJS))

all: $(OBJS)
	ar rc ../../${BUILD_DIR}/${BIN_DIR}/io.a $(OBJS)

../../${BUILD_DIR}/${OBJ_DIR}/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
