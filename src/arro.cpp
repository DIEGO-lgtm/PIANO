///c/Users/rayle/Downloads/piano_arrolladora

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
#include <map> // Usado para los niveles de dificultad

// Definir M_PI si no está definido
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- CONFIGURACIÓN DEL JUEGO ---
const int SCREEN_WIDTH = 700;
const int SCREEN_HEIGHT = 500;
const int NUM_COLUMNS = 4; // Aumentado a 4
const float COLUMN_WIDTH = static_cast<float>(SCREEN_WIDTH) / NUM_COLUMNS;
const float TILE_HEIGHT = 80.f;

const int SAMPLE_RATE = 44100;
const float DURATION_SECONDS = 0.3f;
const int AMPLITUDE = 20000;

const float FALL_DISTANCE = 600.f; // distancia que caen las piezas (ajusta al tamaño de tu pantalla)
const float FALL_SPEED = 300.f;    // pixeles por segundo (ajusta velocidad)
const float FALL_TIME = FALL_DISTANCE / FALL_SPEED;

// --- ESTRUCTURAS Y ESTADOS ---
enum GameState
{
    SHOWING_MENU,
    PLAYING,
    GAME_OVER
};
enum Difficulty
{
    EASY,
    MEDIUM,
    HARD
};

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

// --- SONIDO ---
// Frecuencias de las notas (Escala de Do mayor)
const float FREQ_C4 = 261.63f;
const float FREQ_D4 = 293.66f;
const float FREQ_E4 = 329.63f;
const float FREQ_F4 = 349.23f;
const float FREQ_G4 = 392.00f; // Nueva nota
const float FREQ_A4 = 440.00f; // Nueva nota

sf::SoundBuffer bufferC4, bufferD4, bufferE4, bufferF4, bufferG4, bufferA4;

bool generateSineWave(sf::SoundBuffer &buffer, float frequency)
{
    std::vector<sf::Int16> samples;
    samples.resize(static_cast<size_t>(SAMPLE_RATE * DURATION_SECONDS));
    for (size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] = static_cast<sf::Int16>(AMPLITUDE * std::sin(2 * M_PI * frequency * i / SAMPLE_RATE));
    }
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

void setupGlobalSoundBuffers()
{
    generateSineWave(bufferC4, FREQ_C4);
    generateSineWave(bufferD4, FREQ_D4);
    generateSineWave(bufferE4, FREQ_E4);
    generateSineWave(bufferF4, FREQ_F4);
    generateSineWave(bufferG4, FREQ_G4);
    generateSineWave(bufferA4, FREQ_A4);
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

// Teclas: A, S, D, F, J, K
char getCharForColumn(int column)
{
    switch (column)
    {
    case 0:
        return 'A';
    case 1:
        return 'S';
    case 2:
        return 'k';
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

// --- FUNCIÓN PARA CENTRAR TEXTO ---
void centerOrigin(sf::Text &text)
{
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
}

int main()
{

    srand(static_cast<unsigned int>(time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Piano Tiles Avanzado");
    window.setFramerateLimit(60);

    setupGlobalSoundBuffers();

    // Sonidos para las 6 teclas
    std::vector<sf::Sound> keySounds;
    for (int i = 0; i < NUM_COLUMNS; ++i)
    {
        keySounds.emplace_back(getBufferForColumn(i));
    }

    // --- NIVELES DE DIFICULTAD ---
    std::map<Difficulty, DifficultySettings> difficulties;
    difficulties[EASY] = {150.f, 1.5f};
    difficulties[MEDIUM] = {250.f, 1.2f};
    difficulties[HARD] = {400.f, 0.9f};
    Difficulty currentDifficulty = MEDIUM;

    // --- ESTADO DEL JUEGO ---
    GameState currentState = SHOWING_MENU;
    sf::Clock clock;
    float spawnTimer = 0.f;
    int score = 0;

    int starsEarned = 0;
    sf::Texture starTexture;
    if (!starTexture.loadFromFile("assets/images/estrella.png"))
    {
        std::cerr << "Error al cargar la imagen de estrella." << std::endl;
        return 1;
    }
    std::vector<sf::Sprite> starSprites;
    std::vector<Tile> activeTiles;

    std::vector<float> beatTimes;
    size_t beatIndex = 0;

    // Reloj y música
    sf::Clock musicClock;
    sf::Music music;
    if (!music.openFromFile("assets/sounds/medium_song.WAV"))
    {
        std::cerr << "Error al cargar medium_song.WAV\n";
        return 1;
    }

    // --- Temporizadores para el efecto de iluminación por columna ---
    std::vector<float> keyFlashTimers(NUM_COLUMNS, 0.f);
    const float FLASH_DURATION = 0.2f; // duración del flash en segundos

    // --- RECURSOS GRÁFICOS ---
    sf::Font font;
    if (!font.loadFromFile("assets/Bangers-Regular.ttf"))
    {
        std::cerr << "Error: No se pudo cargar la fuente 'Bangers-Regular.ttf'." << std::endl;
        return 1;
    }

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

    // Escalar la imagen para que ocupe toda la ventana
    menuBackgroundSprite.setScale(
        float(SCREEN_WIDTH) / menuBackgroundTexture.getSize().x,
        float(SCREEN_HEIGHT) / menuBackgroundTexture.getSize().y);

    // Textos del Menú
    sf::Text titleText("KeysRush", font, 55);
    centerOrigin(titleText);
    titleText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 4.f);

    sf::Text promptText("Selecciona tu nivel:", font, 25);
    centerOrigin(promptText);
    promptText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f - 20);

    // Texto debajo (subtítulo o indicación)
    sf::Text subtitleText("(presiona el numero que deseas)", font, 20);
    centerOrigin(subtitleText);
    subtitleText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 20);

    sf::Text easyText("1. Lets Ride Away (Avicci)", font, 20);
    centerOrigin(easyText);
    easyText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 55);

    sf::Text mediumText("2. Arsonist (NOME)", font, 20);
    centerOrigin(mediumText);
    mediumText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 95);

    sf::Text hardText("3. Woops (Dimitri vegas & Like mike )", font, 20);
    centerOrigin(hardText);
    hardText.setPosition(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f + 135);

    // Textos del Juego
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
        float dt = clock.restart().asSeconds();

        sf::Event event;
        // === BUCLE PRINCIPAL ===
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
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
                        // Reiniciar estado para un nuevo juego
                        score = 0;
                        starsEarned = 0;
                        starSprites.clear();
                        activeTiles.clear();
                        spawnTimer = 0;

                        // --- REINICIAR BEATS ---
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
                            keyFlashTimers[pressedColumn] = FLASH_DURATION; // Activar flash
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

                                    // Agregar estrella cada 100 puntos
                                    int newStarsEarned = score / 100;
                                    while (starsEarned < newStarsEarned)
                                    {
                                        sf::Sprite newStar;
                                        newStar.setTexture(starTexture);
                                        newStar.setScale(0.05f, 0.05f);                                     // Ajusta según tamaño real de la imagen
                                        newStar.setPosition(SCREEN_WIDTH - 50.f * (starsEarned + 1), 10.f); // Las coloca alineadas a la derecha
                                        starSprites.push_back(newStar);
                                        starsEarned++;
                                    }

                                    hit = true;
                                    break;
                                }
                            }
                        }
                        // Si se presiona una tecla correcta pero no se acierta a ninguna tile en la zona
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

        // --- LÓGICA DEL JUEGO ---
        if (currentState == PLAYING)
        {

            // ...dentro de if (currentState == PLAYING)...
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
            // ...existing code...
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
            // ...existing code...

            // Mover y comprobar tiles
            for (auto &tile : activeTiles)
            {
                if (tile.active)
                {
                    tile.shape.move({0.f, TILE_SPEED * dt});
                    tile.text.move({0.f, TILE_SPEED * dt});

                    // Si una tile pasa de la pantalla, es game over
                    if (tile.shape.getPosition().y > SCREEN_HEIGHT)
                    {
                        currentState = GAME_OVER;
                        break; // Salir del bucle para no procesar más lógica
                    }
                }
            }

            // Limpiar tiles inactivas (acertadas)
            activeTiles.erase(std::remove_if(activeTiles.begin(), activeTiles.end(),
                                             [](const Tile &t)
                                             { return !t.active; }),
                              activeTiles.end());

            scoreText.setString("Puntaje: " + std::to_string(score));
            // Reducir duración de los flashes
            for (auto &timer : keyFlashTimers)
            {
                if (timer > 0.f)
                    timer -= dt;
                if (music.getStatus() == sf::Music::Stopped && beatIndex >= beatTimes.size())
                {
                    currentState = GAME_OVER;
                }
            }
        }

        // --- DIBUJADO ---
        window.clear(sf::Color(50, 50, 70));

        switch (currentState)
        {
        case SHOWING_MENU:
            window.draw(menuBackgroundSprite); // Dibujar imagen de fondo
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
            // Dibujar efecto de flash por columna
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
            for (const auto &star : starSprites)
            {
                window.draw(star);
            }
            break;

        case GAME_OVER:
            // Dibuja el último estado del juego detrás del texto de Game Over
            window.draw(menuBackgroundSprite); // Dibujar imagen de fondo
            for (const auto &line : columnLines)
                window.draw(line);
            window.draw(targetZone);
            for (const auto &tile : activeTiles)
                window.draw(tile.shape);
            for (const auto &tile : activeTiles)
                window.draw(tile.text);
            window.draw(scoreText);
            for (const auto &star : starSprites)
            {
                window.draw(star);
            }

            // Superpone el mensaje de fin
            window.draw(gameOverText);
            window.draw(restartText);
            break;
        }

        window.display();
    }

    return 0;
}
