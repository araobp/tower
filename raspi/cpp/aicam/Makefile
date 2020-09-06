CXX = g++
CXXFLAGS = -O2 -Wall
CXXFLAGS += `pkg-config --cflags opencv`
LDFLAGS += `pkg-config --libs opencv`
CXXFLAGS += -pthread
CXXFLAGS += -I./tflite/include
LDFLAGS += -L./tflite/lib
LDFLAGS += -ltensorflow-lite

SRCS = $(wildcard src/*.cpp)

HEADERS = $(wildcard src/*.hpp)

OBJECTS = $(patsubst src/%.cpp,bin/%.o,$(SRCS))

OBJS = $(wildcard bin/*.o)

bin/aicam: $(OBJECTS)
	  $(CXX) $^ $(CXXFLAGS) $(LDFLAGS) -o $@

bin/%.o: src/%.cpp $(HEADERS)
	  $(CXX) $< $(CXXFLAGS) $(LDFLAGS) -c -o $@

clean:
	  $(RM) $(OBJS) aicam 
