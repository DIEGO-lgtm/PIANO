CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/opt/homebrew/opt/sfml@2/include
LDFLAGS = -L/opt/homebrew/opt/sfml@2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC = src/arro.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = piano

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
