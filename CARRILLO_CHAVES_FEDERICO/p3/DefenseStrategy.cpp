// ###### Config options ################

#define PRINT_DEFENSE_STRATEGY 1    // generate map images

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef CUSTOM_RAND_GENERATOR
RAND_TYPE SimpleRandomGenerator::a;
#endif

#include <queue>

using namespace Asedio;

#include "cronometro.h"

// Estructura que representa una celda candidata
// valor - valor de la celda
// vector3 - coordenadas de la celda en Vector3
// Celda(int valor, Vector3 vector3) - Constructor
class Celda {
public:
    double valor_;
    Vector3 vector3_;

    Celda() = default;

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
/*
// Algoritmo basado en el pseudocódigo del
// libro "Introduction to Algorithms 3era edicion página 31"
void merge(std::vector<Celda> &V, int p, int q, int r) {
    int n1 = q - p + 1;
    int n2 = r - q;
    std::vector<Celda> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) {
        L[i].valor_ = V[p + i].valor_;
    }
    for (int j = 0; j < n2; ++j) {
        R[j].valor_ = V[q + j + 1].valor_;
    }
    //L[n1] = INT_MAX;
    //R[n2] = INT_MAX;
    int i = 0;
    int j = 0;
    int z = p;
    while (i < n1 && j < n2) {
        //for (int k = p; k < r; ++k) {
        if (L[i].valor_ >= R[j].valor_) { // ahora ordena de forma decreciente.
            V[z].valor_ = L[i].valor_;
            i++;
        } else {
            V[z].valor_ = R[j].valor_;
            j++;
        }
        z++;
    }
    while (i < n1) {
        V[z].valor_ = L[i].valor_;
        i++;
        z++;
    }
    while (j < n2) {
        V[z].valor_ = R[j].valor_;
        j++;
        z++;
    }
}

// Algoritmo basado en el pseudocódigo del
// libro "Introduction to Algorithms 3era edicion página 31"
void mergeSort(std::vector<Celda> &V, int p, int r) {
    if (p < r) {
        int q = (p + r) / 2;
        mergeSort(V, p, q);
        mergeSort(V, q + 1, r);
        merge(V, p, q, r);
    }
}

// Algoritmo basado en el pseudocódigo del
// libro "Introduction to Algorithms 3era edicion página 171"
int partition(std::vector<Celda> &V, int p, int r) {
    double x = V[r].valor_;
    int i = p - 1;
    for (int j = p; j <= r - 1; ++j) {
        if (V[j].valor_ >= x) { // ahora ordena de forma decreciente.
            i = i + 1;
            std::swap(V[i], V[j]);
        }
    }
    std::swap(V[i + 1], V[r]);
    return i + 1;
}

// Algoritmo basado en el pseudocódigo del
// libro "Introduction to Algorithms 3era edicion página 171"
void quickSort(std::vector<Celda> &V, int p, int r) {
    if (p < r) {
        int q = partition(V, p, r);
        quickSort(V, p, q - 1);
        quickSort(V, q + 1, r);
    }
}
*/

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

float
defaultCellValue(int row, int col, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                 std::list<Object *> obstacles) {

    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;

    //Vector3 cellPosition((col * cellWidth) + cellWidth * 0.5f, (row * cellHeight) + cellHeight * 0.5f, 0);
    Vector3 cellPosition = cellCenterToPosition(row, col, cellWidth, cellHeight);

    float val = 0;
    for (auto &obstacle : obstacles) {
        val += _distance(cellPosition, obstacle->position);
    }

    return val;
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

std::vector<Celda>::iterator maximo(std::vector<Celda> &v) {
    std::vector<Celda>::iterator celda;
    double aux = -99999.0;
    for (auto it = v.begin(); it != v.end(); it++) {
        if (it->valor_ > aux) {
            aux = it->valor_;
            celda = it;
        }
    }
    return celda;
}

void devoradorMonticulo(int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                        const std::list<Object *>& obstacles, std::list<Defense *> defenses) {
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
                    defaultCellValue(i, j, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles),
                    aux_vector3);
            Q.push(aux);
        }
    }

    std::vector<Defense *> defensesPlaced;
    auto defensaCandidata = defenses.begin();

    while (defensaCandidata != defenses.end()) {
        Celda celda = Q.top();
        Q.pop();
        (*defensaCandidata)->position = celda.vector3_;
        if (factibilidad(defensesPlaced, mapWidth, mapHeight, (*defensaCandidata), obstacles)) {
            defensesPlaced.push_back((*defensaCandidata));
            ++defensaCandidata;
        }
    }

}

void devoradorSinOrdenar(bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                         const std::list<Object *> &obstacles, const std::list<Defense *> &defenses, float cellWidth,
                         float cellHeight) {
    std::vector<Celda> celdas_candidatas;
    Vector3 aux_vector3;
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux = Celda(
                    defaultCellValue(i, j, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles),
                    aux_vector3);
            celdas_candidatas.push_back(aux);
        }
    }

    //auto max = std::max_element(celdas_candidatas.begin(), celdas_candidatas.end(),
    //                            [](const Celda &a, const Celda &b) {
    //                                return a.valor_ < b.valor_;
    //                            });

    std::vector<Defense *> defensesPlaced;
    auto defensaCandidata = defenses.begin();

    while (defensaCandidata != defenses.end() && !celdas_candidatas.empty()) {
        auto celda = maximo(celdas_candidatas);
        celdas_candidatas.erase(celda);
        (*defensaCandidata)->position = celda->vector3_;
        if (factibilidad(defensesPlaced, mapWidth, mapHeight, (*defensaCandidata), obstacles)) {
            defensesPlaced.push_back((*defensaCandidata));
            defensaCandidata++;
        }
    }
}

void DEF_LIB_EXPORTED
placeDefenses3(bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
               const List<Object *> obstacles, const List<Defense *> defenses) {

    float cellWidth;
    cellWidth = mapWidth / (float) nCellsWidth;
    float cellHeight;
    cellHeight = mapHeight / (float) nCellsHeight;

    if (defenses.size() <= 15 && mapWidth <= 500) {
        devoradorSinOrdenar(freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, cellWidth,
                            cellHeight);
    } else {
        devoradorMonticulo(nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses);
    }


}