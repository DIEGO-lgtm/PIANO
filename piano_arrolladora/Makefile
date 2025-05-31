CXX = g++
CXXFLAGS = -std=c++17 -Wall
SFML_CFLAGS = -I/opt/homebrew/include
SFML_LDFLAGS = -L/opt/homebrew/lib
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC = src/arro.cpp
OUT = piano

all:
	$(CXX) $(CXXFLAGS) $(SFML_CFLAGS) $(SRC) -o $(OUT) $(SFML_LDFLAGS) $(SFML_LIBS)

clean:
	rm -f $(OUT)