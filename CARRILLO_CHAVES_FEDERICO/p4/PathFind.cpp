// ###### Config options ################

#define PRINT_PATHS 1

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef PRINT_PATHS

#include "ppm.h"

#endif

#include <queue>
#include <algorithm>


using namespace Asedio;

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

// this is an strucure which implements the
// operator overlading 
struct ComparaValor {
    bool operator()(AStarNode *nodo1, AStarNode *nodo2) {
        return nodo1->F < nodo2->F;
    }
};

void recupera(const AStarNode *originNode, std::list<Vector3> &path, AStarNode *cur);

std::priority_queue<AStarNode, std::vector<AStarNode>, ComparaValor> Q; // cola de prioridad para las celdas candidatas
double dManhattan(const Vector3 &a, const Vector3 &b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}


void DEF_LIB_EXPORTED
calculateAdditionalCost(float **additionalCost, int cellsWidth, int cellsHeight, float mapWidth, float mapHeight,
                        List<Object *> obstacles, List<Defense *> defenses) {

    float cellWidth = mapWidth / float(cellsWidth);
    float cellHeight = mapHeight / float(cellsHeight);

    for (int i = 0; i < cellsHeight; ++i) {
        for (int j = 0; j < cellsWidth; ++j) {
            Vector3 cellPosition = cellCenterToPosition(i, j, cellWidth, cellHeight);
            float cost = 0;
            if ((i + j) % 2 == 0) {
                cost = cellWidth * 100;
            }

            additionalCost[i][j] = cost;
        }
    }
}

void DEF_LIB_EXPORTED
calculatePath(AStarNode *originNode, AStarNode *targetNode, int cellsWidth, int cellsHeight, float mapWidth,
              float mapHeight, float **additionalCost, std::list<Vector3> &path) {

    float cellWidth = mapWidth / float(cellsWidth);
    float cellHeight = mapHeight / float(cellsHeight);

    std::vector<AStarNode *> opened;
    std::vector<AStarNode *> closed;
    

    AStarNode *cur = originNode;
    opened.push_back(cur);
    std::make_heap(opened.begin(), opened.end());

    bool target = false;
    while (!target && !opened.empty()) {
        cur = opened.front();
        std::pop_heap(opened.begin(), opened.end(), ComparaValor());
        opened.pop_back();
        closed.push_back(cur);

        if (cur == targetNode) {
            target = true;
            path.push_front(cur->position);
        } else {
            double d;
            for (auto j = cur->adjacents.begin(); j != cur->adjacents.end(); ++j) {
                if (closed.end() == std::find(closed.begin(), closed.end(), (*j))) {
                    if (!(opened.end() == std::find(opened.begin(), opened.end(), (*j)))) {
                        d = _distance(cur->position, (*j)->position);
                        if (cur->G + d < (*j)->G) {
                            (*j)->parent = cur;
                            (*j)->G = cur->G + float(d);
                            (*j)->F = (*j)->G + (*j)->H;
                        }
                    } else {
                        int pos_x = int((*j)->position.x / cellWidth);
                        int pos_y = int((*j)->position.y / cellHeight);
                        (*j)->parent = cur;
                        (*j)->G = cur->G +
                                  _distance(cur->position, (*j)->position)
                                  + additionalCost[pos_x][pos_y];
                        (*j)->H = (float) dManhattan((*j)->position, targetNode->position);
                        (*j)->F = (*j)->G + (*j)->H;
                        opened.push_back(*j);
                        std::push_heap(opened.begin(), opened.end(), ComparaValor());
                    }
                }
            }
            std::sort_heap(opened.begin(), opened.end(), ComparaValor());
        }
    }

    recupera(originNode, path, cur);
}

void recupera(const AStarNode *originNode, std::list<Vector3> &path, AStarNode *cur) {
    while (cur->parent != originNode) {
        cur = cur->parent;
        path.push_front(cur->position);
    }
}