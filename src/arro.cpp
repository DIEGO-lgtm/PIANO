/// cd /c/Users/rayle/Downloads/piano_arrolladora (para meterte al directorio del proyecto)

#include <SFML/Graphics.hpp> // Biblioteca SFML para gráficos (ventanas, formas, texto, etc.)
#include <SFML/Audio.hpp>    // Biblioteca SFML para manejar audio (sonido, buffers, música)
#include <vector>            // Contenedor dinámico tipo arreglo
#include <cmath>             // Funciones matemáticas (como seno)
#include <cstdint>           // Tipos de datos enteros con tamaño fijo
#include <cstdlib>           // Funciones como rand(), srand()
#include <ctime>             // Para usar time(), útil para inicializar rand()
#include <string>            // Soporte para strings (cadenas de texto)
#include <fstream>           // Leer y escribir archivos
#include <algorithm>         // Funciones como sort, find, etc.
#include <iostream>          // Entrada/salida (cout, cin)
#include <map>

struct Nota {
    int tiempo_ms;
    int columna;
    bool mostrada = false;
};
               // Contenedor tipo mapa, útil para los niveles de dificultad

// Define PI si no esta definido 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Configuracion del proyecto
const int SCREEN_WIDTH = 700; // Ancho de la ventana (osea el tamaño de la pantalla a lo ancho
const int SCREEN_HEIGHT = 500; // Alto de la ventana (osea el tamaño de la pantalla a lo largo)
const int NUM_COLUMNS = 4; // Número de columnas del juego (al momento de inciar la partida cuantas columnas habra)
const float COLUMN_WIDTH = static_cast<float>(SCREEN_WIDTH) / NUM_COLUMNS; // Ancho de cada columna (se calcula automaticamente dividiendo el ancho de la pantalla por el numero de columnas )
const float TILE_HEIGHT = 80.f; // Alto de cada ficha que cae

// Configuracion del audio 
const int SAMPLE_RATE = 44100; //Muestras por segundo (calidad de audio)
const float DURATION_SECONDS = 0.3f; // Duración de cada nota (al tocar una tecla, cuanto dura el sonido)
const int AMPLITUDE = 20000; // Volumen de la nota

// Configuracion de cada caida de las teclas
const float FALL_DISTANCE = 600.f; // Distancia que recorre una ficha al caer
const float FALL_SPEED = 300.f; // Velocidad de caída (pixeles por segundo)
const float FALL_TIME = FALL_DISTANCE / FALL_SPEED; // Tiempo que tarda una ficha en caer (esto depende de la distancia y la velocidad )

// Estado del juego (osea si esta jugando, en el menu o si se acabo el juego)
enum GameState
{
    SHOWING_MENU, // Mostrando menú (inicio del juego al momento de inciar la partida)
    PLAYING, // Jugando (cuando el jugadador esta en partida)
    GAME_OVER // Juego terminado (osea cuando el jugador a perdido) este estado se muestra cuando el jugador pierde
};

// Los niveles de dificultad del juego (facil, medio, dificil)
enum Difficulty
{
    EASY, // Fácil
    MEDIUM, // Medio
    HARD// Difícil
};

// La configuración de cada nivel de dificultad (osea la velocidad de las fichas y el intervalo entre fichas)
struct DifficultySettings
{
    float tileSpeed; // Velocidad de caída de fichas
    float spawnInterval; // Intervalo entre aparición de fichas
};

// La estructura de una ficha (osea la forma, la letra, la columna y si esta activa o no)
struct Tile
{
    sf::RectangleShape shape; // Forma rectangular de la ficha
    sf::Text text;            // Letra que muestra la ficha (ej. A, S, K, L)
    int column;               // En qué columna está la ficha (0-3)
    bool active = true;       // Si está activa o ya fue tocada
};

// Notas musicales y sus frecuencias (en Hz) solo son 4 ya que solo esas aparecen en el juego
const float FREQ_C4 = 261.63f; // Nota DO
const float FREQ_D4 = 293.66f; // Nota RE
const float FREQ_E4 = 329.63f; // Nota MI
const float FREQ_F4 = 349.23f; // Nota FA

// buffers de sonido para cada nota (C4, D4, E4, F4, G4, A4) esto sirve para almacenar los sonidos de cada nota musical
sf::SoundBuffer bufferC4, bufferD4, bufferE4, bufferF4, bufferG4, bufferA4;

// Función para generar una onda seno y cargarla en un buffer de sonido esto es para  generar el sonido de las notas musicales
bool generateSineWave(sf::SoundBuffer &buffer, float frequency)
{
    std::vector<sf::Int16> samples; // Vector que contendrá las muestras de audio osea para almacenar los datos de audio
    samples.resize(static_cast<size_t>(SAMPLE_RATE * DURATION_SECONDS)); // Tamaño = duración × tasa de muestreo

    // Generar las muestras que son los datos de audio
    for (size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] = static_cast<sf::Int16>(
            AMPLITUDE * std::sin(2 * M_PI * frequency * i / SAMPLE_RATE) // Fórmula de una onda seno para generar el sonido
        );
    }

    // Cargar los datos generados al buffer para que se pueda reproducir
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
}

// Cargar todas las notas musicales en sus respectivos buffers de sonido para que se puedan reproducir al tocar las teclas
void setupGlobalSoundBuffers()
{
    generateSineWave(bufferC4, FREQ_C4);
    generateSineWave(bufferD4, FREQ_D4);
    generateSineWave(bufferE4, FREQ_E4);
    generateSineWave(bufferF4, FREQ_F4);

}

// Obtener el buffer de sonido correspondiente a cada columna (esto es para que al tocar una tecla se reproduzca el sonido de la nota correspondiente)
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
        return bufferA4; // Valor por defecto en caso de error osea si la columna no es valida
    }
}

// Obetener el caracter correspondiente a cada columna (esto es para mostrar la letra de la tecla en la ficha)
char getCharForColumn(int column)
{
    switch (column)
    {
    case 0:
        return 'A'; // Columna 0 → tecla A y esta es la tecla que se toca 
    case 1:
        return 'S'; // Columna 1 → tecla S y esta es la tecla que se toca
    case 2:
        return 'K'; // Columna 2 → tecla K y esta es la tecla que se toca 
    case 3:
        return 'L'; // Columna 3 → tecla L y esta es la tecla que se toca 
    default:
        return 'K'; // Valor por defecto osea si la columna no es valida
    }
}

// Obtener tecla del teclado correspondiente a cada columna (esto es para que al tocar una tecla se pueda detectar la entrada del usuario)
sf::Keyboard::Key getKeyForColumn(int column)
{
    switch (column)
    {
    case 0:
        return sf::Keyboard::A; // es para que al tocar la tecla A se detecte la entrada del usuario
    case 1:
        return sf::Keyboard::S; //  es para que al tocar la tecla S se detecte la entrada del usuario
    case 2:
        return sf::Keyboard::K; // es para que al tocar la tecla K se detecte la entrada del usuario
    case 3:
        return sf::Keyboard::L; // es para que al tocar la tecla L se detecte la entrada del usuario
    default:
        return sf::Keyboard::K; // Valor por defecto osea si la columna no es valida
    }
}

//  Centrar el origen de un texto (esto es para que al mostrar el texto en pantalla este centrado)
void centerOrigin(sf::Text &text) // Esta funcion es para centrar el origen del texto
{
    sf::FloatRect bounds = text.getLocalBounds(); // Obtener el tamaño del texto osea el ancho y alto del texto
    text.setOrigin(bounds.width / 2.f, bounds.height / 2.f); // Establecer el origen en el centro osea que el texto se muestre centrado
}

int main() // Función principal del programa, donde se inicia el juego y se maneja la lógica principal
{

    std::vector<Nota> notas;
    std::ifstream archivo("hard_beats.txt");
    int t, c;
    while (archivo >> t >> c) {
        notas.push_back({t, c});
    }

    sf::Clock reloj;


    srand(static_cast<unsigned int>(time(nullptr))); // Inicializar la semilla para números aleatorios (esto es para que los numeros aleatorios sean diferentes cada vez que se ejecuta el programa)
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Piano Tiles Avanzado"); // Crear la ventana del juego con el tamaño especificado y el título
    window.setFramerateLimit(60); // Limitar la tasa de fotogramas a 60 FPS (esto es para que el juego no vaya demasiado rapido y se vea fluido)

    setupGlobalSoundBuffers(); // Cargar los buffers de sonido para las notas musicales (esto es para que al tocar las teclas se reproduzca el sonido de la nota correspondiente)

    // Sonidos para las 6 teclas
    std::vector<sf::Sound> keySounds; // Vector para almacenar los sonidos de cada tecla (esto es para que al tocar una tecla se reproduzca el sonido de la nota correspondiente)
    for (int i = 0; i < NUM_COLUMNS; ++i) // Iterar sobre cada columna osea es para  que al tocar una tecla se reproduzca el sonido de la nota correspondiente 
    {
        keySounds.emplace_back(getBufferForColumn(i)); // Crear un sonido para cada columna usando el buffer correspondiente (esto es para que al tocar una tecla se reproduzca el sonido de la nota correspondiente)
    }

    // Son para los niveles de dificultad osea para que al seleccionar un nivel de dificultad se establezcan las configuraciones correspondientes
    std::map<Difficulty, DifficultySettings> difficulties; // Mapa que asocia cada nivel de dificultad con sus configuraciones
    difficulties[EASY] = {150.f, 1.5f}; // Configuración para el nivel fácil (velocidad de las fichas y el intervalo entre fichas)
    difficulties[MEDIUM] = {250.f, 1.2f}; // Configuración para el nivel medio (velocidad de las fichas y el intervalo entre fichas)
    difficulties[HARD] = {400.f, 0.9f}; // Configuración para el nivel difícil (velocidad de las fichas y el intervalo entre fichas)
    Difficulty currentDifficulty = MEDIUM; // Nivel de dificultad actual (se inicia en medio)

    // Este es el estado del juego (osea si esta en el menu, jugando o si se acabo el juego)
    GameState currentState = SHOWING_MENU; // Esto es para que al iniciar el juego se muestre el menú
    sf::Clock clock; // Esto es  para medir el tiempo transcurrido en el juego (osea para que al iniciar el juego se pueda medir el tiempo que ha pasado desde que se inicio la partida)
    float spawnTimer = 0.f; // Temporizador para controlar la aparición de nuevas fichas (esto es para que al iniciar el juego se pueda controlar la aparición de nuevas fichas en el juego)
    int score = 0; // Puntuación del jugador (esto es para que al iniciar el juego se pueda llevar un registro de la puntuación del jugador)
    int starsEarned = 0; // Estrellas ganadas (esto es para que al finalizar el juego se pueda mostrar cuantas estrellas ha ganado el jugador)

    sf::Texture starTexture; // Textura para las estrellas (esto es para que al finalizar el juego se pueda mostrar las estrellas ganadas por el jugador)
    if (!starTexture.loadFromFile("assets/images/estrella.png")) // Cargar la textura de la estrella (esto es para que al finalizar el juego se pueda mostrar las estrellas ganadas por el jugador)
    {
        std::cerr << "Error al cargar la imagen de estrella." << std::endl; // Si no se puede cargar la textura, mostrar un mensaje de error
        return 1; // Terminar el programa con un error
    }
    std::vector<sf::Sprite> starSprites; // Vector para almacenar los sprites de las estrellas (esto es para que al finalizar el juego se pueda mostrar las estrellas ganadas por el jugador)
    std::vector<Tile> activeTiles; // Vector para almacenar las fichas activas (esto es para que al iniciar el juego se pueda llevar un registro de las fichas que estan activas en el juego)

    std::vector<float> beatTimes;  // Vector para almacenar los tiempos de los beats (esto es para que al iniciar el juego se pueda llevar un registro de los tiempos de los beats de la canción)
    size_t beatIndex = 0; // Índice del beat actual (esto es para que al iniciar el juego se pueda llevar un registro del beat actual de la canción)

    // Reloj y música
    sf::Clock musicClock; // Reloj para la música (esto es para que al iniciar el juego se pueda llevar un registro del tiempo que ha pasado desde que se inicio la música)
    sf::Music music; // Objeto de música para reproducir la canción (esto es para que al iniciar el juego se pueda reproducir la música de fondo)
    if (!music.openFromFile("assets/sounds/medium_song.WAV")) // Cargar la música desde un archivo (esto es para que al iniciar el juego se pueda reproducir la música de fondo)
    {
        std::cerr << "Error al cargar medium_song.WAV\n"; // Si no se puede cargar la música, mostrar un mensaje de error
        return 1;
    }

    // Temporizadores para el flash de las teclas (esto es para que al tocar una tecla se muestre un efecto visual de flash)
    std::vector<float> keyFlashTimers(NUM_COLUMNS, 0.f); // Vector para almacenar los temporizadores de flash de cada tecla (esto es para que al tocar una tecla se muestre un efecto visual de flash)
    const float FLASH_DURATION = 0.2f; // duración del flash en segundos

    // Recursos graficos osea imagenes y fuentes de texto
    sf::Font font; // fuente para el texto osea que es para mostrar el texto en la pantalla
    if (!font.loadFromFile("assets/Bangers-Regular.ttf")) // Cargar la fuente desde un archivo (esto es para que al iniciar el juego se pueda mostrar el texto en la pantalla)
    {
        std::cerr << "Error: No se pudo cargar la fuente 'Bangers-Regular.ttf'." << std::endl; // Si no se puede cargar la fuente, mostrar un mensaje de error
        return 1;
    }

    sf::Texture menuBackgroundTexture; // Textura para el fondo del menú (esto es para que al iniciar el juego se pueda mostrar el fondo del menú)
    if (!menuBackgroundTexture.loadFromFile("assets/images/menu_background.png")) // Cargar la textura del fondo del menú (esto es para que al iniciar el juego se pueda mostrar el fondo del menú)
    {
        std::cerr << "Error al cargar la imagen de fondo del menú." << std::endl; // Si no se puede cargar la textura, mostrar un mensaje de error
        return 1;
    }
    sf::Texture congratsTexture; // Textura para la imagen de felicitaciones (esto es para que al finalizar el juego se pueda mostrar una imagen de felicitaciones)
    if (!congratsTexture.loadFromFile("assets/images/congrats.png")) // Cargar la textura de la imagen de felicitaciones (esto es para que al finalizar el juego se pueda mostrar una imagen de felicitaciones)
    {
        std::cerr << "Error al cargar la imagen de felicitaciones." << std::endl; // Si no se puede cargar la textura, mostrar un mensaje de error
    }
    sf::Sprite congratsSprite; // Sprite para la imagen de felicitaciones (esto es para que al finalizar el juego se pueda mostrar una imagen de felicitaciones)
    congratsSprite.setTexture(congratsTexture); // Establecer la textura del sprite de felicitaciones (esto es para que al finalizar el juego se pueda mostrar una imagen de felicitaciones)
    congratsSprite.setScale( // Escalar la imagen de felicitaciones para que ocupe toda la ventana
        float(SCREEN_WIDTH) / congratsTexture.getSize().x, // es calar el ancho de la imagen de felicitaciones
        float(SCREEN_HEIGHT) / congratsTexture.getSize().y); // es calar el alto de la imagen de felicitaciones

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

        // --- Sincronización de notas con el tiempo ---
        float tiempo_actual = reloj.getElapsedTime().asMilliseconds();
        for (auto& nota : notas) {
            if (!nota.mostrada && tiempo_actual >= nota.tiempo_ms - 1500) {
                // Aquí se debe crear la tecla y posicionarla según nota.columna
                // Por ejemplo: crearTecla(nota.columna);
                nota.mostrada = true;
            }
        }

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