#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <algorithm>
#include <iostream>

// Definir M_PI si no está definido
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int NUM_COLUMNS = 4;
const float COLUMN_WIDTH = static_cast<float>(SCREEN_WIDTH) / NUM_COLUMNS;
const float TILE_HEIGHT = 100.f;
const float TILE_SPEED = 250.f;
const float SPAWN_INTERVAL = 1.2f;

const int SAMPLE_RATE = 44100;
const float DURATION_SECONDS = 0.3f;
const int AMPLITUDE = 20000;

struct Tile {
    sf::RectangleShape shape;
    sf::Text text;
    int column;
    bool active = true;
    sf::Sound sound;

    // SFML 2.6.2 doesn't require font in the constructor here if we set it later
    Tile(const sf::SoundBuffer& buffer) : sound(buffer) {}
};

// Updated function signature for sf::SoundBuffer::loadFromSamples in SFML 2.6.2
bool generateSineWave(sf::SoundBuffer& buffer, float frequency) {
    std::vector<sf::Int16> samples;
    samples.resize(static_cast<size_t>(SAMPLE_RATE * DURATION_SECONDS));
    for (size_t i = 0; i < samples.size(); ++i) {
        samples[i] = static_cast<sf::Int16>(AMPLITUDE * std::sin(2 * M_PI * frequency * i / SAMPLE_RATE));
    }
    // The version with channelMap is from a newer API. This is the 2.6.2 compatible way.
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

sf::SoundBuffer bufferC4, bufferD4, bufferE4, bufferF4;

void setupGlobalSoundBuffers() {
    generateSineWave(bufferC4, 261.63f);
    generateSineWave(bufferD4, 293.66f);
    generateSineWave(bufferE4, 329.63f);
    generateSineWave(bufferF4, 349.23f);
}

sf::SoundBuffer& getBufferForColumn(int column) {
    switch (column) {
        case 0: return bufferC4;
        case 1: return bufferD4;
        case 2: return bufferE4;
        case 3: default: return bufferF4;
    }
}

char getCharForColumn(int column) {
    switch (column) {
        case 0: return 'A';
        case 1: return 'S';
        case 2: return 'D';
        case 3: default: return 'F';
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::String windowTitle = "Piano Tiles SFML (Teclas A, S, D, F)";
    // Updated sf::VideoMode constructor for older SFML versions
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), windowTitle);
    window.setFramerateLimit(60);

    setupGlobalSoundBuffers();

    sf::Sound keyPressSoundA(bufferC4);
    sf::Sound keyPressSoundS(bufferD4);
    sf::Sound keyPressSoundD(bufferE4);
    sf::Sound keyPressSoundF(bufferF4);

    std::vector<Tile> activeTiles;
    sf::Clock clock;
    float spawnTimer = 0.f;
    int score = 0;

    sf::Font font;
    bool fontLoaded = false;

    // Use loadFromFile instead of openFromFile
    if (font.loadFromFile("arial.ttf")) {
        fontLoaded = true;
        std::cout << "Fuente 'arial.ttf' cargada exitosamente desde el directorio local." << std::endl;
    } else {
        // Use std::string instead of std::filesystem::path
        std::string systemFontPath = "/System/Library/Fonts/Helvetica.ttc"; // macOS path
        // You might want to add a path for Windows like "C:/Windows/Fonts/arial.ttf"
        if (font.loadFromFile(systemFontPath)) {
           fontLoaded = true;
           std::cout << "Fuente del sistema '" << systemFontPath << "' cargada exitosamente." << std::endl;
        } else {
            std::cerr << "Error: No se pudo cargar la fuente 'arial.ttf' ni la fuente del sistema '" << systemFontPath << "'." << std::endl;
            std::cerr << "Asegúrate de que 'arial.ttf' esté en el mismo directorio que el ejecutable o que la ruta de la fuente del sistema sea válida." << std::endl;
        }
    }

    sf::Text scoreText;
    if(fontLoaded) scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10.f, 10.f);
    scoreText.setString("Puntaje: 0");

    sf::RectangleShape targetZone(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), TILE_HEIGHT / 2));
    targetZone.setFillColor(sf::Color(255, 255, 255, 50));
    targetZone.setPosition(0.f, SCREEN_HEIGHT - TILE_HEIGHT * 1.5f);

    sf::Text gameOverText;
    if(fontLoaded) gameOverText.setFont(font);
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("FIN DEL JUEGO");
    if (fontLoaded) {
        // Use left, top, width, height for sf::FloatRect in SFML 2
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    }
    gameOverText.setPosition(static_cast<float>(SCREEN_WIDTH) / 2.0f, static_cast<float>(SCREEN_HEIGHT) / 2.0f);
    bool gameOver = false;

    sf::Text restartText;
    if(fontLoaded) restartText.setFont(font);
    restartText.setCharacterSize(20);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Presiona R para reiniciar");
    if (fontLoaded) {
        // Use left, top, width, height for sf::FloatRect in SFML 2
        sf::FloatRect restartTextRect = restartText.getLocalBounds();
        restartText.setOrigin(restartTextRect.left + restartTextRect.width / 2.0f, restartTextRect.top + restartTextRect.height / 2.0f);
    }
    restartText.setPosition(static_cast<float>(SCREEN_WIDTH) / 2.0f, static_cast<float>(SCREEN_HEIGHT) / 2.0f + 50.f);

    std::vector<sf::VertexArray> columnLines(NUM_COLUMNS - 1);
    for (int i = 0; i < NUM_COLUMNS - 1; ++i) {
        columnLines[i].setPrimitiveType(sf::Lines);
        columnLines[i].append(sf::Vertex(sf::Vector2f(COLUMN_WIDTH * (i + 1), 0.f), sf::Color(100, 100, 100)));
        columnLines[i].append(sf::Vertex(sf::Vector2f(COLUMN_WIDTH * (i + 1), static_cast<float>(SCREEN_HEIGHT)), sf::Color(100, 100, 100)));
    }

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Classic SFML 2 event loop
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (!gameOver) {
                if (event.type == sf::Event::KeyPressed) {
                    int pressedColumn = -1;
                    if (event.key.code == sf::Keyboard::A) {
                        pressedColumn = 0;
                        if(keyPressSoundA.getStatus() != sf::Sound::Playing) keyPressSoundA.play();
                    } else if (event.key.code == sf::Keyboard::S) {
                        pressedColumn = 1;
                        if(keyPressSoundS.getStatus() != sf::Sound::Playing) keyPressSoundS.play();
                    } else if (event.key.code == sf::Keyboard::D) {
                        pressedColumn = 2;
                        if(keyPressSoundD.getStatus() != sf::Sound::Playing) keyPressSoundD.play();
                    } else if (event.key.code == sf::Keyboard::F) {
                        pressedColumn = 3;
                        if(keyPressSoundF.getStatus() != sf::Sound::Playing) keyPressSoundF.play();
                    }

                    if (pressedColumn != -1) {
                        bool hit = false;
                        for (auto& tile : activeTiles) {
                            if (tile.active && tile.column == pressedColumn) {
                                sf::FloatRect tileBounds = tile.shape.getGlobalBounds();
                                sf::FloatRect targetBounds = targetZone.getGlobalBounds();
                                // Use intersects() instead of findIntersection().has_value()
                                if (tileBounds.intersects(targetBounds)) {
                                    tile.active = false;
                                    score += 10;
                                    hit = true;
                                    break;
                                }
                            }
                        }
                        if (!hit && !activeTiles.empty()) {
                             // gameOver = true;
                        }
                    }
                }
            } else { // if (gameOver)
                 if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        activeTiles.clear();
                        score = 0;
                        spawnTimer = 0.f;
                        gameOver = false;
                        scoreText.setString("Puntaje: " + std::to_string(score));
                    }
                }
            }
        }


        if (!gameOver) {
            spawnTimer += dt;
            if (spawnTimer >= SPAWN_INTERVAL) {
                spawnTimer = 0.f;
                int newColumn = rand() % NUM_COLUMNS;
                Tile newTile(getBufferForColumn(newColumn));
                newTile.column = newColumn;
                newTile.shape.setSize(sf::Vector2f(COLUMN_WIDTH - 2.f, TILE_HEIGHT));
                newTile.shape.setFillColor(sf::Color::Black);
                newTile.shape.setOutlineColor(sf::Color::White);
                newTile.shape.setOutlineThickness(1.f);
                newTile.shape.setPosition(COLUMN_WIDTH * newTile.column + 1.f, -TILE_HEIGHT);

                if (fontLoaded) {
                    newTile.text.setFont(font); // Set font for the new tile's text
                    newTile.text.setString(std::string(1, getCharForColumn(newColumn)));
                    newTile.text.setCharacterSize(static_cast<unsigned int>(TILE_HEIGHT * 0.6f));
                    newTile.text.setFillColor(sf::Color::White);
                    // Use left, top, width, height for sf::FloatRect in SFML 2
                    sf::FloatRect tileTextBounds = newTile.text.getLocalBounds();
                    newTile.text.setOrigin(tileTextBounds.left + tileTextBounds.width / 2.f,
                                            tileTextBounds.top + tileTextBounds.height / 2.f);

                    newTile.text.setPosition(newTile.shape.getPosition().x + newTile.shape.getSize().x / 2.f,
                                              newTile.shape.getPosition().y + newTile.shape.getSize().y / 2.f);
                }
                activeTiles.push_back(newTile);
            }

            for (size_t i = 0; i < activeTiles.size(); ++i) {
                if (activeTiles[i].active) {
                    activeTiles[i].shape.move(0.f, TILE_SPEED * dt);
                    if (fontLoaded) {
                        activeTiles[i].text.move(0.f, TILE_SPEED * dt);
                    }
                    if (activeTiles[i].shape.getPosition().y > SCREEN_HEIGHT) {
                        activeTiles[i].active = false;
                        // gameOver = true;
                    }
                }
            }

            activeTiles.erase(std::remove_if(activeTiles.begin(), activeTiles.end(),
                                            [](const Tile& t){ return !t.active; }),
                            activeTiles.end());

            scoreText.setString("Puntaje: " + std::to_string(score));
        }

        window.clear(sf::Color(50, 50, 70));

        for (const auto& line : columnLines) {
            window.draw(line);
        }

        window.draw(targetZone);

        for (const auto& tile : activeTiles) {
            if (tile.active) {
                 window.draw(tile.shape);
                 if (fontLoaded) {
                     window.draw(tile.text);
                 }
            }
        }

        if (fontLoaded) {
            window.draw(scoreText);
            if (gameOver) {
                window.draw(gameOverText);
                window.draw(restartText);
            }
        }

        window.display();
    }

    return 0;
}