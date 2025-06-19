#pragma doce // es pa
// Estado del juego (osea si esta jugando, en el menu o si se acabo el juego)
enum GameState
{
    SHOWING_START,
    SHOWING_MENU, // Mostrando menú (inicio del juego al momento de inciar la partida)
    PLAYING, // Jugando (cuando el jugadador esta en partida)
    GAME_OVER // Juego terminado (osea cuando el jugador a perdido) este estado se muestra cuando el jugador pierde
};
