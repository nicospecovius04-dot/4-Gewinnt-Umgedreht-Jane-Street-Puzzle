CXX = clang++

CXXFLAGS = -Wall -Wextra -std=c++23 -IC:\msys64\ucrt64\include

LDFLAGS = -LC:\msys64\ucrt64\lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = $(wildcard 01src/*.cpp)
OBJ = $(patsubst 01src/%.cpp,build/%.o,$(SRC))

TARGET = build/test.exe

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: 01src/%.cpp	
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./build/test.exe

clean:
	rm -f build/*.o build/*.exe
