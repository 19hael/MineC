CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = microc
SOURCES = main.cpp lexer.cpp parser.cpp compiler.cpp vm.cpp debugger.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(TARGET)

test: $(TARGET)
	./$(TARGET) Examples/test.mc

debug: $(TARGET)
	./$(TARGET) Examples/test.mc --debug
