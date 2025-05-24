#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <map>
#include <string>

using namespace std;

const int N = 12;
const int SIZE = 800;
const int CELL = SIZE / N;

// Clase Mapa
class Mapa {
public:
    int n, size;
    vector<vector<char>> Mat;

    Mapa(int _n, int _size, vector<vector<char>> mat) : n(_n), size(_size), Mat(mat) {}

    void editarMapa(string direcc, int posx, int posy) {
        int i = posy / (size / n);
        int j = posx / (size / n);

        if (direcc == "W" && i > 0) i--;
        else if (direcc == "A" && j > 0) j--;
        else if (direcc == "S" && i < n - 1) i++;
        else if (direcc == "D" && j < n - 1) j++;

        if (Mat[i][j] == 'h') return;
        if (Mat[i][j] == 'O') Mat[i][j] = '-';
        else if (Mat[i][j] == '-') Mat[i][j] = 'O';
    }
};

// Clase Jugador
class Jugador {
public:
    int x, y, size, n, velocidad;
    string ultimaTecla;

    Jugador(int _x, int _y, int _size, int _n, int _velocidad)
        : x(_x), y(_y), size(_size), n(_n), velocidad(_velocidad) {}

    void mover(sf::Keyboard::Key tecla, const vector<vector<char>>& Mat) {
        int i = y / (size / n);
        int j = x / (size / n);
        if (tecla == sf::Keyboard::W && i > 0 && Mat[i - 1][j] != 'h') {
            y -= velocidad;
            ultimaTecla = "W";
        } else if (tecla == sf::Keyboard::S && i < n - 1 && Mat[i + 1][j] != 'h') {
            y += velocidad;
            ultimaTecla = "S";
        } else if (tecla == sf::Keyboard::A && j > 0 && Mat[i][j - 1] != 'h') {
            x -= velocidad;
            ultimaTecla = "A";
        } else if (tecla == sf::Keyboard::D && j < n - 1 && Mat[i][j + 1] != 'h') {
            x += velocidad;
            ultimaTecla = "D";
        }
    }


    void ajustarPosicion() {
        x = ((x + CELL / 2) / CELL) * CELL;
        y = ((y + CELL / 2) / CELL) * CELL;
    }

    void especial(Mapa &mapa) {
        mapa.editarMapa(ultimaTecla, x, y);
    }
};

// Clase Villano
class Villano {
public:
    int x, y, velocidad;

    Villano(int _x, int _y, int _velocidad) : x(_x), y(_y), velocidad(_velocidad) {}
};

// Clase Item
class Item {
public:
    int x, y;
    bool visible;

    Item(int _x, int _y) : x(_x), y(_y), visible(true) {}
};

// A* Nodo
struct Nodo {
    int x, y;
    int g, h;
    Nodo *padre;
    int f() const { return g + h; }
};

vector<pair<int, int>> vecinos = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

int heuristica(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

bool dentro(int x, int y) {
    return x >= 0 && x < N && y >= 0 && y < N;
}

pair<int, int> A_estrella(Mapa &mapa, int sx, int sy, int dx, int dy) {
    vector<vector<bool>> visitado(N, vector<bool>(N, false));
    priority_queue<pair<int, Nodo*>, vector<pair<int, Nodo*>>, greater<>> pq;

    Nodo *inicio = new Nodo{sx, sy, 0, heuristica(sx, sy, dx, dy), nullptr};
    pq.push({inicio->f(), inicio});

    while (!pq.empty()) {
        Nodo *actual = pq.top().second; pq.pop();
        if (visitado[actual->x][actual->y]) continue;
        visitado[actual->x][actual->y] = true;

        if (actual->x == dx && actual->y == dy) {
            while (actual->padre && actual->padre->padre)
                actual = actual->padre;
            return {actual->x, actual->y};
        }

        for (auto &[dx, dy] : vecinos) {
            int nx = actual->x + dx, ny = actual->y + dy;
            if (dentro(nx, ny) && !visitado[nx][ny] && mapa.Mat[nx][ny] != 'h') {
                int costo = (mapa.Mat[nx][ny] == 'O') ? 2 : 1;
                Nodo *nuevo = new Nodo{nx, ny, actual->g + costo, heuristica(nx, ny, dx, dy), actual};
                pq.push({nuevo->f(), nuevo});
            }
        }
    }
    return {sx, sy};
}

// Crear el mapa
void crearMapa(sf::RenderWindow &ventana, int size) {
    vector<vector<char>> matriz = {
        {'h','h','h','h','h','h','h','h','h','h','h','h'},
        {'h','-','-','-','-','h','-','-','-','-','-','h'},
        {'h','-','-','-','-','h','-','-','-','-','-','h'},
        {'h','-','-','h','-','h','-','h','h','-','-','h'},
        {'h','-','-','h','-','h','-','h','-','-','-','h'},
        {'h','-','-','h','-','h','-','h','-','-','-','h'},
        {'h','-','-','h','-','-','-','h','-','-','-','h'},
        {'h','-','-','h','-','-','-','h','-','-','-','h'},
        {'h','-','-','h','-','h','-','h','-','-','-','h'},
        {'h','-','-','h','-','-','-','-','-','-','-','h'},
        {'h','-','-','h','-','-','-','-','-','-','-','h'},
        {'h','h','h','h','h','h','h','h','h','h','h','h'},
    };

    Mapa mapa(N, size, matriz);
    Jugador jugador(CELL, CELL, size, N, CELL);
    Villano villano(size - 2 * CELL, size - 2 * CELL, CELL);
 
    Item item((N-2)*CELL, (N-2)*CELL);

    sf::Texture texH, texItem, texPiso;
    texH.loadFromFile("h.png");
    texPiso.loadFromFile("piso.png");
    texItem.loadFromFile("item.png");
    sf::Sprite spriteH(texH);
    sf::Sprite spritePiso(texPiso);
    spriteH.setScale((float)CELL / texH.getSize().x, (float)CELL / texH.getSize().y);
    spritePiso.setScale((float)CELL / texPiso.getSize().x, (float)CELL / texPiso.getSize().y);

    sf::Texture texJugador, texVillano;
    texJugador.loadFromFile("jugador.png");
    texVillano.loadFromFile("villano.png");
    sf::Sprite sprJugador(texJugador), sprVillano(texVillano), sprItem(texItem);
    sprJugador.setScale((float)CELL / texJugador.getSize().x, (float)CELL / texJugador.getSize().y);
    sprVillano.setScale((float)CELL / texVillano.getSize().x, (float)CELL / texVillano.getSize().y);
    sprItem.setScale((float)CELL / texItem.getSize().x, (float)CELL / texItem.getSize().y);

    sf::Clock reloj;
    float tiempo = 0;

    while (ventana.isOpen()) {
        sf::Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed)
                ventana.close();
            if (evento.type == sf::Event::KeyPressed) {
                if (evento.key.code == sf::Keyboard::P)
                    jugador.especial(mapa);
                else
                    jugador.mover(evento.key.code,matriz);
            }
        }

        tiempo += reloj.restart().asSeconds();
        if (tiempo >= 1.0f) {
            tiempo = 0;
            auto [nx, ny] = A_estrella(mapa, villano.y / CELL, villano.x / CELL, jugador.y / CELL, jugador.x / CELL);
            villano.x = ny * CELL;
            villano.y = nx * CELL;
        }

        jugador.ajustarPosicion();

        if (jugador.x == item.x && jugador.y == item.y && item.visible) {
            item.visible = false;
            sf::Texture final;
            final.loadFromFile("final.png");
            sf::Sprite finalSprite(final);
            ventana.clear();
            ventana.draw(finalSprite);
            ventana.display();
            sf::sleep(sf::seconds(3));
            ventana.close();
        }
        if (jugador.x == villano.x && jugador.y == villano.y) {
            sf::Texture gameover;
            gameover.loadFromFile("gameover.png");
            sf::Sprite gameoverSprite(gameover);
            ventana.clear();
            ventana.draw(gameoverSprite);
            ventana.display();
            sf::sleep(sf::seconds(3));
            ventana.close();
        }


        ventana.clear();

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (matriz[i][j] == 'h') {
                    spriteH.setPosition(j * CELL, i * CELL);
                    ventana.draw(spriteH);
                }
                else if (matriz[i][j] == '-') {
                    spritePiso.setPosition(j * CELL, i * CELL);
                    ventana.draw(spritePiso);
                }
            }
        }

        sprJugador.setPosition(jugador.x, jugador.y);
        sprVillano.setPosition(villano.x, villano.y);
        if (item.visible) sprItem.setPosition(item.x, item.y);

        ventana.draw(sprJugador);
        ventana.draw(sprVillano);
        if (item.visible) ventana.draw(sprItem);

        ventana.display();
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(SIZE, SIZE), "mapa1");

    sf::Texture texInicio;
    texInicio.loadFromFile("inicio.png");
    sf::Sprite sprInicio(texInicio);

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();
            if (e.type == sf::Event::MouseButtonPressed)
                crearMapa(window, SIZE);
        }

        window.clear();
        window.draw(sprInicio);
        window.display();
    }

    return 0;
}