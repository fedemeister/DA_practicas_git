// ###### Config options ################

#define PRINT_DEFENSE_STRATEGY 1    // generate map images

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef PRINT_DEFENSE_STRATEGY

#include "ppm.h"

#endif

#ifdef CUSTOM_RAND_GENERATOR
RAND_TYPE SimpleRandomGenerator::a;
#endif

#include <queue>

using namespace Asedio;

// Estructura que representa una celda candidata
// valor - valor de la celda
// vector3 - coordenadas de la celda en Vector3
// Celda(int valor, Vector3 vector3) - Constructor
class Celda {
public:
    double valor_;
    Vector3 vector3_;

    Celda(double valor, const Vector3 &vector3) : valor_(valor), vector3_(vector3) {}
};

// this is an strucure which implements the
// operator overlading 
struct ComparaValor {
    bool operator()(Celda const &celda1, Celda const &celda2) {
        // return "true" if "celda1" is ordered  
        // before "celda2", for example: 
        return celda1.valor_ > celda2.valor_;
    }
};

struct ComparaValor2 {
    bool operator()(Celda const &celda1, Celda const &celda2) {
        // return "true" if "celda1" is ordered  
        // before "celda2", for example: 
        return celda1.valor_ < celda2.valor_;
    }
};


// Devuelve la posición en el mapa del centro de la celda (i,j)
// i - fila
// j - columna
// cellWidth - ancho de las celdas
// cellHeight - alto de las celdas
Vector3 cellCenterToPosition(int i, int j, float cellWidth, float cellHeight) {
    return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0);
}

// Devuelve la celda a la que corresponde una posición en el mapa
// pos - posición que se quiere convertir
// i_out - fila a la que corresponde la posición pos (resultado)
// j_out - columna a la que corresponde la posición pos (resultado)
// cellWidth - ancho de las celdas
// cellHeight - alto de las celdas
void positionToCell(const Vector3 &pos, int &i_out, int &j_out, float cellWidth, float cellHeight) {
    i_out = (int) (pos.y * 1.0f / cellHeight);
    j_out = (int) (pos.x * 1.0f / cellWidth);
}


struct ComparaRadioDefensas {
    bool operator()(Defense const &first, Defense const &second) {
        // return "true" if "celda1" is ordered
        // before "celda2", for example:
        return first.radio < second.radio;
    }
};

// idea de https://en.cppreference.com/w/cpp/algorithm/all_any_none_of
struct choca {
    Defense *mi_defensa;
    Object *objeto;

    explicit choca(Object *objeto, Defense *mi_defensa) : objeto(objeto),
                                                          mi_defensa(mi_defensa) {}

    bool operator()(Object *object) const {
        return
                (_distance(object->position, mi_defensa->position) <=
                 object->radio + (mi_defensa->radio));
    }
};

int cellValue(int row, int col, bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                 const std::list<Object *> &obstacles, std::list<Defense *> defenses, bool esCEM) {
    // implemente aqui la función que asigna valores a las celdas
    float cellWidth = mapWidth / float(nCellsWidth);
    float cellHeight = mapHeight / float(nCellsHeight);
    if (esCEM) {
        Vector3 pos_actual = cellCenterToPosition(row, col, cellWidth, cellHeight);
        Vector3 centroDelMapa_v3 = Vector3(mapWidth / 2, mapHeight / 2, 0);
        return abs(_distance(pos_actual, centroDelMapa_v3));
    } else {
        int i_out;
        int j_out;
        std::list<Defense *>::const_iterator ci_CEM = defenses.begin();
        Vector3 aux_vector3;
        positionToCell((*ci_CEM)->position, i_out, j_out, cellWidth, cellHeight);
        aux_vector3 = cellCenterToPosition(row - i_out, col - j_out, cellWidth, cellHeight);
        return std::max(mapWidth, mapHeight) - aux_vector3.length();
    }
}

bool factibilidad(const std::vector<Defense *> &defensasColocadas, float mapWidth, float mapHeight, Defense *mi_defensa,
                  const std::list<Object *> &obstacles) {
    Object *obstaculo = nullptr;
    Object *defensa = nullptr;

    // Dentro de los bordes del mapa
    if (mi_defensa->position.x + mi_defensa->radio <= mapWidth && (mi_defensa->position.x - mi_defensa->radio) >= 0 &&
        (mi_defensa->position.y + mi_defensa->radio) <= mapHeight &&
        (mi_defensa->position.y - mi_defensa->radio) >= 0) {}
    else {
        return false;
    }

    // No colisiona con defensas colocadas
    if (std::any_of(defensasColocadas.cbegin(), defensasColocadas.cend(), choca(defensa, mi_defensa))) {
        return false;
    }

    //No colisiona con ningún obstáculo
    if (std::any_of(obstacles.cbegin(), obstacles.cend(), choca(obstaculo, mi_defensa))) {
        return false;
    }

    return true;
}


void DEF_LIB_EXPORTED
placeDefenses(bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
              std::list<Object *> obstacles, std::list<Defense *> defenses) {

    float cellWidth;
    cellWidth = mapWidth / (float) nCellsWidth;
    float cellHeight;
    cellHeight = mapHeight / (float) nCellsHeight;

    std::priority_queue<Celda, std::vector<Celda>, ComparaValor> Q; // cola de prioridad para las celdas candidatas

    Vector3 aux_vector3;
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux = Celda(
                    cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses,
                              true),
                    aux_vector3);
            Q.push(aux);
        }
    }

    std::vector<Defense *> defensesPlaced;
    auto defensaCandidata = defenses.begin();
    bool CEM_colocado = false;

    // CEM
    while (defensaCandidata != defenses.end() && !CEM_colocado) {
        Celda celda = Q.top();
        Q.pop();
        (*defensaCandidata)->position = celda.vector3_;
        if (factibilidad(defensesPlaced, mapWidth, mapHeight, (*defensaCandidata), obstacles)) {
            defensesPlaced.push_back((*defensaCandidata));
            ++defensaCandidata;
            CEM_colocado = true;
        }
    }

    // demás defensas
    std::priority_queue<Celda, std::vector<Celda>, ComparaValor2> Q2;   // nueva cola de prioridad
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux_celda = Celda(
                    cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses,
                              false),
                    aux_vector3);
            Q2.push(aux_celda);
        }
    }
    //defenses.sort([](const Defense *a, const Defense *b) { return a->health < b->heath; });

    while (defensaCandidata != defenses.end()) {
        Celda celda = Q2.top();
        Q2.pop();
        (*defensaCandidata)->position = celda.vector3_;
        if (factibilidad(defensesPlaced, mapWidth, mapHeight, (*defensaCandidata), obstacles)) {
            defensesPlaced.push_back((*defensaCandidata));
            ++defensaCandidata;
        }
    }

#ifdef PRINT_DEFENSE_STRATEGY

    float **cellValues = new float *[nCellsHeight];
    for (int i = 0; i < nCellsHeight; ++i) {
        cellValues[i] = new float[nCellsWidth];
        for (int j = 0; j < nCellsWidth; ++j) {
            cellValues[i][j] =
                    ((int) (cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles,
                                      defenses, false))) % 256;
        }
    }
    dPrintMap("strategy.ppm", nCellsHeight, nCellsWidth, cellHeight, cellWidth, freeCells,
              cellValues, std::list<Defense *>(), false);

    for (int i = 0; i < nCellsHeight; ++i)
        delete[] cellValues[i];
    delete[] cellValues;
    cellValues = nullptr;

#endif
}
