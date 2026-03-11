LIB_DIR := ../lib

CXX := g++
# (-I adds LIB_DIR to compiler search path (inside files, no need 4 ../../lib/))
FLAGS := -std=c++11 -O3 -lm 
#-I$(LIB_DIR)

LIB_SRCS := $(wildcard $(LIB_DIR)/*.cpp)
LOCAL_SRCS := $(wildcard *.cpp)

LOCAL_HEADERS := $(wildcard *.hpp) $(wildcard $(LIB_DIR)/*.hpp)

# convert .cpp to .o filenames
LIB_OBJS := $(LIB_SRCS:.cpp=.o)
LOCAL_OBJS := $(LOCAL_SRCS:.cpp=.o)
ALL_OBJS := $(LIB_OBJS) $(LOCAL_OBJS)

all: main

# (what runs when you type 'make')
main: $(ALL_OBJS)
	$(CXX) $(FLAGS) $^ -o $@

# build a .o file from a .cpp file (for the release build)
%.o: %.cpp $(LOCAL_HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

# --- cleanup ---
clean:
	rm -f main debug_main *.o $(LIB_DIR)/*.o results.txt
