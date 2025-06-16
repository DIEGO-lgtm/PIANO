CXX = g++
CXXFLAGS = -std=c++17 -Wall -I"C:/msys64/mingw64/include"
LDFLAGS = -L"C:/msys64/mingw64/lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC = src/arro.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = piano

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
