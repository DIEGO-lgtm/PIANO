#ifndef ARRO_HPP
#define ARRO_HPP

#include <SFML/Audio.hpp>
#include <map>

// Reproduce una nota de acuerdo con la tecla presionada
void reproducirNota(char tecla, sf::Sound& sound, const std::map<char, sf::SoundBuffer>& buffers);

// Carga los sonidos en un mapa
bool cargarSonidos(std::map<char, sf::SoundBuffer>& buffers);

#endif // ARRO_HPP