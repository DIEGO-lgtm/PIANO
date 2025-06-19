# 🎵 Piano Tiles - SFML Game

Juego rítmico estilo **Piano Tiles** hecho en **C++** con la biblioteca **SFML**, donde debes presionar las teclas correctas al ritmo de la música para obtener puntos y estrellas.

---

## 🚀 Características

- Beats sincronizados con música
- Diferentes niveles de dificultad (Fácil, Medio, Difícil)
- Sistema de puntuación con estrellas
- Sonidos únicos por tecla
- Interfaz visual con texto, sprites y efectos

---

## 📂 Estructura del Proyecto

```
.
├── assets/             # Archivos de sonido, imágenes y fuentes
├── src/
│   └── arro.cpp        # Código fuente principal
├── Makefile            # (Opcional en Windows)
└── README.md           # Este archivo
```

---

## 🧩 Requisitos

- C++17 o superior
- SFML 2.6.2
- Compilador: g++, clang++ o MSVC
- Git

---

## 🖥️ Instalación en macOS

1. Instala [Homebrew](https://brew.sh/) si no lo tienes.
2. Instala SFML:
   ```bash
   brew install sfml@2
   ```
3. Clona el repositorio:
   ```bash
   git clone https://github.com/tu_usuario/piano-tiles-sfml.git
   cd piano-tiles-sfml
   ```
4. Compila:
   ```bash
   make
   ```
5. Ejecuta el juego:
   ```bash
   ./piano
   ```

---

## 🪟 Instalación en Windows

### Opción A: MSYS2 + MinGW64

1. Instala [MSYS2](https://www.msys2.org/).
2. Abre **MSYS2 MinGW64** y ejecuta:
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SFML make git
   ```
3. Clona el proyecto:
   ```bash
   git clone https://github.com/tu_usuario/piano-tiles-sfml.git
   cd piano-tiles-sfml
   ```
4. Compila:
   ```bash
   make
   ```
5. Ejecuta:
   ```bash
   ./piano.exe
   ```

> Asegúrate de que las DLLs de SFML estén en el mismo directorio que `piano.exe`.

### Opción B: Visual Studio

1. Instala [Visual Studio](https://visualstudio.microsoft.com/) con el paquete **Desarrollo de escritorio con C++**.
2. Descarga [SFML 2.6.2 para Visual Studio](https://www.sfml-dev.org/download.php).
3. Configura tu proyecto:
   - Agrega `src/arro.cpp`
   - Configura rutas de `Include` y `Library`
   - Copia las DLLs necesarias al directorio de salida
4. Compila y ejecuta.

---

## 🛠️ Makefile de ejemplo

Para macOS con Homebrew:

```make
CXX = g++
CXXFLAGS = -std=c++17 -Wall
INCLUDES = -I/opt/homebrew/opt/sfml@2/include
LIBS = -L/opt/homebrew/opt/sfml@2/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC = src/arro.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = piano

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LIBS)

clean:
	rm -f $(OBJ) $(TARGET)
```

Para MSYS2, cambia `INCLUDES` y `LIBS` según corresponda (`/mingw64/include`, `/mingw64/lib`).

---

## 📖 Manual de Usuario

### 🎮 Objetivo del juego

Presiona las teclas correctas sincronizadas con el ritmo de la música para sumar puntos. Si fallas muchas veces, el juego termina. ¡Acumula estrellas y supera tu récord!

### ⌨️ Controles

| Tecla | Acción                      |
|-------|-----------------------------|
| A     | Tocar la columna izquierda  |
| S     | Tocar la columna central-izquierda |
| D     | Tocar la columna central-derecha |
| F     | Tocar la columna derecha    |
| ESC   | Salir del juego             |

### 🕹️ Cómo jugar

1. **Inicia el juego** ejecutando el binario (`./piano` o `piano.exe`).
2. **Selecciona una dificultad**: Fácil, Medio o Difícil.
3. Comenzará la canción. **Observa cómo bajan las notas** (tiles).
4. **Presiona la tecla correspondiente** cuando una nota alcance la parte inferior de la pantalla.
5. **Gana puntos y estrellas** por cada nota acertada.
6. **El juego termina** cuando la canción acaba o si fallas demasiadas notas.

### ⭐ Sistema de puntuación

- +10 puntos por nota correcta
- Combo de 10: Gana 1 estrella
- Combo de 20: Gana 2 estrellas
- Fallos: -1 vida por cada nota perdida
- 3 vidas perdidas: Fin del juego

### 🧠 Consejos

- Usa audífonos para una mejor sincronización con la música.
- Comienza con el modo Fácil para practicar.
- Observa el patrón de los tiles para anticiparte.
- ¡No presiones demasiado pronto o tarde!

---

## 👤 Autores
**Diego Pérez 24110241**  
**Raymundo Lecuona* 24110274**