#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int TAMBOR_WIDTH = 100;
const int TAMBOR_HEIGHT = 100;

struct Tile {
    int x, y;
    char key;
    sf::RectangleShape shape;
    bool active;
    bool animating = false;
    sf::Clock pressed_clock;
};

struct Reward {
    bool active = false;
    sf::Clock clock;
};

sf::Text createText(sf::Font& font, const std::string& str, int size = 26, sf::Color color = sf::Color::White) {
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(color);
    return text;
}

int main() {
    std::srand((unsigned)std::time(nullptr));
    sf::RenderWindow window(sf::VideoMode(480, 800), "Piano Arrolladora");
    window.setFramerateLimit(60);

    sf::Texture fondoTex, drumTex, reneTex;
    if (!fondoTex.loadFromFile("assets/images/fondo.png") ||
        !drumTex.loadFromFile("assets/images/tambor.png") ||
        !reneTex.loadFromFile("assets/images/rene.png")) {
        std::cerr << "Error cargando imagenes" << std::endl;
        return 1;
    }

    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Error cargando fuente" << std::endl;
        return 1;
    }

    sf::Music music;
    if (!music.openFromFile("assets/music/tu_cancion.mp3")) {
        std::cerr << "Error cargando musica" << std::endl;
    }

    sf::SoundBuffer golpeBuffer;
    if (!golpeBuffer.loadFromFile("assets/sounds/dump.wav")) {
        std::cerr << "Error cargando dump.wav" << std::endl;
    }
    sf::Sound golpe;
    golpe.setBuffer(golpeBuffer);

    sf::Sprite fondo(fondoTex);
    sf::Sprite rene(reneTex);
    rene.setPosition(20, 10);

    bool inStartScreen = true;
    sf::Text titleText = createText(font, "Piano Arrolladora", 30);
    titleText.setPosition(200 - titleText.getLocalBounds().width / 2, 150);
    sf::Text startText = createText(font, "Presiona ENTER para comenzar");
    startText.setPosition(200 - startText.getLocalBounds().width / 2, 300);

    while (inStartScreen) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) return 0;
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter) {
                inStartScreen = false;
                if (music.getStatus() != sf::Music::Playing)
                    music.play();
            }
        }
        window.clear();
        window.draw(fondo);
        window.draw(titleText);
        window.draw(startText);
        window.display();
    }

    std::vector<Tile> tiles;
    int score = 0;
    Reward reward;
    sf::Clock spawnClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                char pressed = '\0';
                switch (event.key.code) {
                    case sf::Keyboard::A: pressed = 'A'; break;
                    case sf::Keyboard::S: pressed = 'S'; break;
                    case sf::Keyboard::D: pressed = 'D'; break;
                    case sf::Keyboard::F: pressed = 'F'; break;
                    default: break;
                }

                if (pressed != '\0') {
                    for (auto& tile : tiles) {
                        if (tile.active && tile.y >= 450 && tile.y <= 550 && tile.key == pressed) {
                            tile.active = false;
                            tile.animating = true;
                            tile.pressed_clock.restart();
                            score++;
                            golpe.play();
                            std::cout << "¡Tambor tocado! Puntos: " << score << std::endl;
                            if (score % 10 == 0) {
                                reward.active = true;
                                reward.clock.restart();
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (spawnClock.getElapsedTime().asMilliseconds() > 1200) {
            int col = rand() % 4;
            char keys[4] = {'A', 'S', 'D', 'F'};
            Tile t;
            t.x = col * 100 + 10;
            t.y = -TAMBOR_HEIGHT;
            t.key = keys[col];
            t.shape.setSize(sf::Vector2f(TAMBOR_WIDTH, TAMBOR_HEIGHT));
            t.shape.setPosition(t.x, t.y);
            t.shape.setTexture(&drumTex);
            t.active = true;
            tiles.push_back(t);
            spawnClock.restart();
        }

        for (auto& tile : tiles) {
            if (tile.active || tile.animating) {
                tile.y += 4;
                tile.shape.setPosition(tile.x, tile.y);
            }
        }

        window.clear();
        window.draw(fondo);

        for (auto& tile : tiles) {
            if (!tile.active && !tile.animating) continue;

            sf::RectangleShape shape = tile.shape;
            if (tile.animating && tile.pressed_clock.getElapsedTime().asMilliseconds() < 150) {
                shape.setSize({TAMBOR_WIDTH + 10.f, TAMBOR_HEIGHT + 10.f});
                shape.setPosition(tile.x - 5, tile.y - 5);
            } else {
                tile.animating = false;
            }
            window.draw(shape);

            sf::Text letter = createText(font, std::string(1, tile.key));
            letter.setPosition(tile.x + TAMBOR_WIDTH/2 - letter.getLocalBounds().width/2,
                               tile.y + TAMBOR_HEIGHT/2 - letter.getLocalBounds().height);
            window.draw(letter);
        }

        // Mostrar letras guía A S D F
        std::string teclas = "A   S   D   F";
        sf::Text letrasGuia = createText(font, teclas, 24);
        letrasGuia.setPosition(200 - letrasGuia.getLocalBounds().width / 2, 370);
        window.draw(letrasGuia);

        if (reward.active && reward.clock.getElapsedTime().asSeconds() < 5) {
            window.draw(rene);
            sf::Text puntos = createText(font, "Puntos: " + std::to_string(score));
            puntos.setPosition(rene.getPosition().x + 100, rene.getPosition().y + 20);
            window.draw(puntos);
        } else {
            reward.active = false;
        }

        window.display();
    }
    return 0;
}
