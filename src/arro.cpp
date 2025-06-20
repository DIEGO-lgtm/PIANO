// cd /c/Users/rayle/Downloads/piano_arrolladora (para meterte al directorio del proyecto)

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <map>

struct Nota
{
    int tiempo_ms;
    int columna;
    bool mostrada = false;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SCREEN_WIDTH = 700;
const int SCREEN_HEIGHT = 500;
const int NUM_COLUMNS = 4;
const float COLUMN_WIDTH = static_cast<float>(SCREEN_WIDTH) / NUM_COLUMNS;
const float TILE_HEIGHT = 80.f;

const int SAMPLE_RATE = 44100;
const float DURATION_SECONDS = 0.3f;
const int AMPLITUDE = 20000;

const float FALL_DISTANCE = 600.f;
const float FALL_SPEED = 300.f;
const float FALL_TIME = FALL_DISTANCE / FALL_SPEED;

#include <GameState.hpp>
#include <Difficulty.hpp>

struct DifficultySettings
{
    float tileSpeed;
    float spawnInterval;
};

struct Tile
{
    sf::RectangleShape shape;
    sf::Text text;
    int column;
    bool active = true;
};

const float FREQ_C4 = 261.63f;
const float FREQ_D4 = 293.66f;
const float FREQ_E4 = 329.63f;
const float FREQ_F4 = 349.23f;

sf::SoundBuffer bufferC4, bufferD4, bufferE4, bufferF4, bufferG4, bufferA4;

bool generateSineWave(sf::SoundBuffer &buffer, float frequency)
{
    std::vector<sf::Int16> samples;
    samples.resize(static_cast<size_t>(SAMPLE_RATE * DURATION_SECONDS));
    for (size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] = static_cast<sf::Int16>(
            AMPLITUDE * std::sin(2 * M_PI * frequency * i / SAMPLE_RATE)
        );
    }
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void setupGlobalSoundBuffers()
{
    generateSineWave(bufferC4, FREQ_C4);
    generateSineWave(bufferD4, FREQ_D4);
    generateSineWave(bufferE4, FREQ_E4);
    generateSineWave(bufferF4, FREQ_F4);
}

sf::SoundBuffer &getBufferForColumn(int column)
{
    switch (column)
    {
    case 0:
        return bufferC4;
    case 1:
        return bufferD4;
    case 2:
        return bufferE4;
    case 3:
        return bufferF4;
    default:
        return bufferA4;
    }
}

char getCharForColumn(int column)
{
    switch (column)
    {
    case 0:
        return 'A';
    case 1:
        return 'S';
    case 2:
        return 'K';
    case 3:
        return 'L';
    default:
        return 'K';
    }
}

sf::Keyboard::Key getKeyForColumn(int column)
{
    switch (column)
    {
    case 0:
        return sf::Keyboard::A;
    case 1:
        return sf::Keyboard::S;
    case 2:
        return sf::Keyboard::K;
    case 3:
        return sf::Keyboard::L;
    default:
        return sf::Keyboard::K;
    }
}

void centerOrigin(sf::Text &text)
{
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

int main()
{
    std::vector<Nota> notas;
    std::ifstream archivo("hard_beats.txt");
    int t, c;
    while (archivo >> t >> c)
    {
        notas.push_back({t, c});
    }

    sf::Clock reloj;

    srand(static_cast<unsigned int>(time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Piano Tiles Avanzado");
    window.setFramerateLimit(60);

    setupGlobalSoundBuffers();

    std::vector<sf::Sound> keySounds;
    for (int i = 0; i < NUM_COLUMNS; ++i)
    {
        keySounds.emplace_back(getBufferForColumn(i));
    }

    std::map<Difficulty, DifficultySettings> difficulties;
    difficulties[EASY] = {150.f, 1.5f};
    difficulties[MEDIUM] = {250.f, 1.2f};
    difficulties[HARD] = {400.f, 0.9f};
    Difficulty currentDifficulty = MEDIUM;

    GameState currentState = SHOWING_START;
    sf::Clock clock;
    float spawnTimer = 0.f;
    int score = 0;
    int starsEarned = 0;

    sf::Font font;
    if (!font.loadFromFile("assets/Orbitron-Regular.ttf"))
    {
        std::cerr << "Error: No se pudo cargar la fuente 'Bangers-Regular.ttf'." << std::endl;
        return 1;
    }
    sf::Texture starTexture;
    if (!starTexture.loadFromFile("assets/images/estrella.png"))
    {
        std::cerr << "Error al cargar la imagen de estrella." << std::endl;
        return 1;
    }

    sf::Sprite starSprite;
    starSprite.setTexture(starTexture);
    starSprite.setScale(0.05f, 0.05f);
    starSprite.setPosition(SCREEN_WIDTH - 70.f, 10.f);
    sf::Text starMultiplierText("x1", font, 28);
    starMultiplierText.setFillColor(sf::Color::Yellow);
    starMultiplierText.setPosition(SCREEN_WIDTH - 140.f, 10.f);

    std::vector<Tile> activeTiles;

    std::vector<float> beatTimes;
    size_t beatIndex = 0;

    sf::Clock musicClock;
    sf::Music music;
    if (!music.openFromFile("assets/sounds/medium_song.WAV"))
    {
        std::cerr << "Error al cargar medium_song.WAV\n";
        return 1;
    }

    std::vector<float> keyFlashTimers(NUM_COLUMNS, 0.f);
    const float FLASH_DURATION = 0.2f;

    sf::Text startText("PRESIONA ENTER PARA INCIAR", font, 35);
    centerOrigin(startText);
    startText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    sf::Texture menuBackgroundTexture;
    if (!menuBackgroundTexture.loadFromFile("assets/images/menu_background.png"))
    {
        std::cerr << "Error al cargar la imagen de fondo del menú." << std::endl;
        return 1;
    }
    sf::Texture congratsTexture;
    if (!congratsTexture.loadFromFile("assets/images/congrats.png"))
    {
        std::cerr << "Error al cargar la imagen de felicitaciones." << std::endl;
    }
    sf::Sprite congratsSprite;
    congratsSprite.setTexture(congratsTexture);
    congratsSprite.setScale(
        float(SCREEN_WIDTH) / congratsTexture.getSize().x,
        float(SCREEN_HEIGHT) / congratsTexture.getSize().y);

    sf::Sprite menuBackgroundSprite;
    menuBackgroundSprite.setTexture(menuBackgroundTexture);
    menuBackgroundSprite.setScale(
        float(SCREEN_WIDTH) / menuBackgroundTexture.getSize().x,
        float(SCREEN_HEIGHT) / menuBackgroundTexture.getSize().y);

    sf::Text titleText("KeysRush", font, 55);
    centerOrigin(titleText);
    titleText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 4.f);

    sf::Text promptText("Selecciona tu nivel:", font, 30);
    centerOrigin(promptText);
    promptText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 20);

    sf::Text subtitleText("(presiona el numero que deseas)", font, 20);
    centerOrigin(subtitleText);
    subtitleText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 20);

    sf::Text easyText("1. Lets Ride Away (Avicci)", font, 25);
    centerOrigin(easyText);
    easyText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 55);

    sf::Text mediumText("2. Arsonist (NOME)", font, 25);
    centerOrigin(mediumText);
    mediumText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 95);

    sf::Text hardText("3. Tremor (Martin garrix)", font, 25);
    centerOrigin(hardText);
    hardText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 135);

    sf::Text scoreText("", font, 24);
    scoreText.setPosition(10.f, 10.f);

    sf::Text gameOverText("FIN DEL JUEGO", font, 50);
    gameOverText.setFillColor(sf::Color::Red);
    centerOrigin(gameOverText);
    gameOverText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f);

    sf::Text restartText("Presiona 'R' para volver al menu", font, 20);
    centerOrigin(restartText);
    restartText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 50.f);

    sf::RectangleShape targetZone(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), TILE_HEIGHT / 2));
    targetZone.setFillColor(sf::Color(255, 255, 255, 50));
    targetZone.setPosition(0.f, SCREEN_HEIGHT - TILE_HEIGHT * 1.5f);

    std::vector<sf::VertexArray> columnLines(NUM_COLUMNS - 1);
    for (int i = 0; i < NUM_COLUMNS - 1; ++i)
    {
        columnLines[i].setPrimitiveType(sf::Lines);
        columnLines[i].append(sf::Vertex(sf::Vector2f(COLUMN_WIDTH * (i + 1), 0.f), sf::Color(100, 100, 100)));
        columnLines[i].append(sf::Vertex(sf::Vector2f(COLUMN_WIDTH * (i + 1), static_cast<float>(SCREEN_HEIGHT)), sf::Color(100, 100, 100)));
    }
    while (window.isOpen())
    {
        float tiempo_actual = reloj.getElapsedTime().asMilliseconds();
        for (auto &nota : notas)
        {
            if (!nota.mostrada && tiempo_actual >= nota.tiempo_ms - 1500)
            {
                nota.mostrada = true;
            }
        }

        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (currentState == SHOWING_START)
            {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                {
                    currentState = SHOWING_MENU;
                }
                continue;
            }

            switch (currentState)
            {
            case SHOWING_MENU:
            {
                window.draw(menuBackgroundSprite);
                bool selectionMade = false;
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Num1)
                    {
                        currentDifficulty = EASY;
                        selectionMade = true;
                    }
                    else if (event.key.code == sf::Keyboard::Num2)
                    {
                        currentDifficulty = MEDIUM;
                        selectionMade = true;
                    }
                    else if (event.key.code == sf::Keyboard::Num3)
                    {
                        currentDifficulty = HARD;
                        selectionMade = true;
                    }

                    if (selectionMade)
                    {
                        score = 0;
                        starsEarned = 0;
                        activeTiles.clear();
                        spawnTimer = 0;
                        beatIndex = 0;
                        beatTimes.clear();
                        std::string beatsFile;
                        std::string musicFile;
                        switch (currentDifficulty)
                        {
                        case EASY:
                            beatsFile = "assets/beats/easy_beats.txt";
                            musicFile = "assets/sounds/easy_song.WAV";
                            break;
                        case MEDIUM:
                            beatsFile = "assets/beats/beats.txt";
                            musicFile = "assets/sounds/medium_song.WAV";
                            break;
                        case HARD:
                            beatsFile = "assets/beats/hard_beats.txt";
                            musicFile = "assets/sounds/hard_song.WAV";
                            break;
                        default:
                            break;
                        }
                        std::ifstream beatFile(beatsFile);
                        float beat;
                        while (beatFile >> beat)
                        {
                            beatTimes.push_back(beat);
                        }

                        if (!music.openFromFile(musicFile))
                        {
                            std::cerr << "Error al cargar " << musicFile << std::endl;
                        }
                        else
                        {
                            music.play();
                        }

                        currentState = PLAYING;
                    }
                }
                break;
            }
            case PLAYING:
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    int pressedColumn = -1;
                    for (int i = 0; i < NUM_COLUMNS; ++i)
                    {
                        if (event.key.code == getKeyForColumn(i))
                        {
                            pressedColumn = i;
                            keyFlashTimers[pressedColumn] = FLASH_DURATION;
                            break;
                        }
                    }

                    if (pressedColumn != -1)
                    {
                        bool hit = false;
                        for (auto &tile : activeTiles)
                        {
                            if (tile.active && tile.column == pressedColumn)
                            {
                                sf::FloatRect tileBounds = tile.shape.getGlobalBounds();
                                if (tileBounds.intersects(targetZone.getGlobalBounds()))
                                {
                                    tile.active = false;
                                    score += 10;
                                    int newStarsEarned = score / 100;
                                    if (newStarsEarned > starsEarned)
                                    {
                                        starsEarned = newStarsEarned;
                                        starMultiplierText.setString("x" + std::to_string(starsEarned));
                                    }
                                    hit = true;
                                    break;
                                }
                            }
                        }
                        if (!hit)
                        {
                            music.stop();
                            currentState = GAME_OVER;
                        }
                    }
                }
                break;
            }
            case GAME_OVER:
            {
                if (music.getStatus() == sf::Music::Playing)
                {
                    music.stop();
                }
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::R)
                    {
                        currentState = SHOWING_MENU;
                    }
                }
                window.draw(congratsSprite);
                break;
            }
            default:
                break;
            }
        }

        if (currentState == PLAYING)
        {
            float TILE_SPEED = difficulties[currentDifficulty].tileSpeed;
            if (currentDifficulty == MEDIUM && beatIndex < beatTimes.size())
            {
                float tiempoCaida = (SCREEN_HEIGHT - TILE_HEIGHT * 1.5f) / TILE_SPEED;
                float musicTime = music.getPlayingOffset().asSeconds();
                while (beatIndex < beatTimes.size() && musicTime >= beatTimes[beatIndex] - tiempoCaida)
                {
                    Tile newTile;
                    newTile.column = rand() % NUM_COLUMNS;
                    newTile.shape.setSize({COLUMN_WIDTH - 2.f, TILE_HEIGHT});
                    newTile.shape.setFillColor(sf::Color::Black);
                    newTile.shape.setOutlineColor(sf::Color::White);
                    newTile.shape.setOutlineThickness(1.f);
                    newTile.shape.setPosition({COLUMN_WIDTH * newTile.column + 1.f, -TILE_HEIGHT});

                    newTile.text.setFont(font);
                    newTile.text.setString(std::string(1, getCharForColumn(newTile.column)));
                    newTile.text.setCharacterSize(static_cast<unsigned int>(TILE_HEIGHT * 0.6f));
                    newTile.text.setFillColor(sf::Color::White);
                    centerOrigin(newTile.text);
                    newTile.text.setPosition({newTile.shape.getPosition().x + newTile.shape.getSize().x / 2.f,
                                              newTile.shape.getPosition().y + newTile.shape.getSize().y / 2.f});

                    activeTiles.push_back(newTile);
                    beatIndex++;
                }
            }
            if (currentDifficulty != MEDIUM)
            {
                float SPAWN_INTERVAL = difficulties[currentDifficulty].spawnInterval;
                spawnTimer += dt;
                if (spawnTimer >= SPAWN_INTERVAL)
                {
                    spawnTimer = 0.f;
                    Tile newTile;
                    newTile.column = rand() % NUM_COLUMNS;
                    newTile.shape.setSize({COLUMN_WIDTH - 2.f, TILE_HEIGHT});
                    newTile.shape.setFillColor(sf::Color::Black);
                    newTile.shape.setOutlineColor(sf::Color::White);
                    newTile.shape.setOutlineThickness(1.f);
                    newTile.shape.setPosition({COLUMN_WIDTH * newTile.column + 1.f, -TILE_HEIGHT});

                    newTile.text.setFont(font);
                    newTile.text.setString(std::string(1, getCharForColumn(newTile.column)));
                    newTile.text.setCharacterSize(static_cast<unsigned int>(TILE_HEIGHT * 0.6f));
                    newTile.text.setFillColor(sf::Color::White);
                    centerOrigin(newTile.text);
                    newTile.text.setPosition({newTile.shape.getPosition().x + newTile.shape.getSize().x / 2.f,
                                              newTile.shape.getPosition().y + newTile.shape.getSize().y / 2.f});

                    activeTiles.push_back(newTile);
                }
            }

            for (auto &tile : activeTiles)
            {
                if (tile.active)
                {
                    tile.shape.move({0.f, TILE_SPEED * dt});
                    tile.text.move({0.f, TILE_SPEED * dt});
                    if (tile.shape.getPosition().y > SCREEN_HEIGHT)
                    {
                        currentState = GAME_OVER;
                        break;
                    }
                }
            }

            activeTiles.erase(std::remove_if(activeTiles.begin(), activeTiles.end(),
                                             [](const Tile &t)
                                             { return !t.active; }),
                              activeTiles.end());

            scoreText.setString("Puntaje: " + std::to_string(score));
            for (auto &timer : keyFlashTimers)
            {
                if (timer > 0.f)
                    timer -= dt;
                if (music.getStatus() == sf::Music::Stopped && beatIndex >= beatTimes.size())
{
    currentState = GAME_WIN;
}
            }
        }

        window.clear(sf::Color(50, 50, 70));
        if (currentState == SHOWING_START)
        {
            window.clear(sf::Color(50, 50, 70));
            window.draw(menuBackgroundSprite);
            window.draw(startText);
            window.display();
            continue;
        }

        switch (currentState)
        {
        case SHOWING_MENU:
            window.draw(menuBackgroundSprite);
            window.draw(titleText);
            window.draw(subtitleText);
            window.draw(promptText);
            window.draw(easyText);
            window.draw(mediumText);
            window.draw(hardText);
            break;

        case PLAYING:
            window.draw(menuBackgroundSprite);
            for (const auto &line : columnLines)
                window.draw(line);
            window.draw(targetZone);
            for (int i = 0; i < NUM_COLUMNS; ++i)
            {
                if (keyFlashTimers[i] > 0.f)
                {
                    sf::RectangleShape flash(sf::Vector2f(COLUMN_WIDTH - 2.f, SCREEN_HEIGHT));
                    flash.setPosition(i * COLUMN_WIDTH + 1.f, 0.f);
                    flash.setFillColor(sf::Color(255, 255, 100, static_cast<sf::Uint8>(200 * (keyFlashTimers[i] / FLASH_DURATION))));
                    window.draw(flash);
                }
            }
            for (const auto &tile : activeTiles)
                window.draw(tile.shape);
            for (const auto &tile : activeTiles)
                window.draw(tile.text);
            window.draw(scoreText);
            if (starsEarned >= 1)
            {
                if (starsEarned > 1)
                    window.draw(starMultiplierText);
                window.draw(starSprite);
            }
            break;

        case GAME_OVER:
            window.draw(menuBackgroundSprite);
            for (const auto &line : columnLines)
                window.draw(line);
            window.draw(targetZone);
            for (const auto &tile : activeTiles)
                window.draw(tile.shape);
            for (const auto &tile : activeTiles)
                window.draw(tile.text);
            window.draw(scoreText);
            if (starsEarned >= 1)
            {
                if (starsEarned > 1)
                    window.draw(starMultiplierText);
                window.draw(starSprite);
            }
            window.draw(gameOverText);
            window.draw(restartText);
            break;
            
    case GAME_WIN:
        window.draw(menuBackgroundSprite);
        window.draw(congratsSprite);
        break;
        }

        window.display();
    }

    return 0;
}