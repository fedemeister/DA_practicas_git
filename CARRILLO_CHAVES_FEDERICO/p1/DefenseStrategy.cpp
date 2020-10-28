// ###### Config options ################

//#define PRINT_DEFENSE_STRATEGY 1    // generate map images

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
struct Celda {
    int valor;
    Vector3 vector3;

    Celda(int valor, const Vector3 &vector3) : valor(valor), vector3(vector3) {}
};

std::vector<Celda> candidatos;

// this is an strucure which implements the 
// operator overlading 
struct ComparaValor {
    bool operator()(Celda const &celda1, Celda const &celda2) {
        // return "true" if "celda1" is ordered  
        // before "celda2", for example: 
        return celda1.valor < celda2.valor;
    }
};

std::priority_queue<Celda, std::vector<Celda>, ComparaValor> Q;


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


bool posicionDentroMapa(const Celda &prometedora, float radio, float mapWidth, float mapHeight) {
    if (prometedora.vector3.x + radio < mapWidth && prometedora.vector3.y + radio < mapHeight &&
        prometedora.vector3.x - radio > 0.0 && prometedora.vector3.y - radio > 0.0) {
        return true;
    } else
        return false;
}

float cellValue(int row, int col, bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
                List<Object *> obstacles, List<Defense *> defenses) {
    return (nCellsWidth-col)*(nCellsHeight-row); // implemente aqui la función que asigna valores a las celdas
}

bool factibilidad(Defense *defense, std::list<Object *> obstacles, std::list<Defense *> defensesPlaced, float mapWidth,
                  float mapHeight) {
    bool factible = true;
    auto currentObstacle = obstacles.begin();
    auto currentDefense = defensesPlaced.begin();

    if (defense->position.x + defense->radio > mapWidth || (defense->position.x - defense->radio) < 0 ||
        (defense->position.y + defense->radio) > mapHeight || (defense->position.y - defense->radio) < 0) {
        factible = false;
    }

    while (currentObstacle != obstacles.end() && factible) {
        if ((_distance(defense->position, (*currentObstacle)->position) -
             (defense->radio + (*currentObstacle)->radio)) < 0) {
            factible = false;
        }
        currentObstacle++;
    }

    while (currentDefense != defensesPlaced.end() && factible) {
        if ((_distance(defense->position, (*currentDefense)->position)
             - (defense->radio + (*currentDefense)->radio)) < 0) {
            factible = false;
        }
        currentDefense++;
    }
    return factible;
}

void DEF_LIB_EXPORTED
placeDefenses(bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
              std::list<Object *> obstacles, std::list<Defense *> defenses) {

    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;


    std::cout << "******************************************************" << std::endl;
    std::cout << "freeCells:" << *freeCells << std::endl;
    std::cout << "nCellsWidth:" << nCellsWidth << std::endl;
    std::cout << "nCellsHeight:" << nCellsHeight << std::endl;
    std::cout << "mapWidth:" << mapWidth << std::endl;
    std::cout << "mapHeight:" << mapHeight << std::endl;
    std::cout << "******************************************************" << std::endl;


    Vector3 aux_vector3;
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux = Celda(cellValue(i,j,freeCells,nCellsWidth,nCellsHeight,mapWidth,mapHeight,obstacles,defenses), aux_vector3);
            Q.push(aux);
        }
    }

    int maxAttemps = 1000;
    auto currentDefense = defenses.begin();
    std::list<Defense *> defensesPlaced;
    //Vector3 vector3 = cellCenterToPosition(5, 3, cellWidth, cellHeight);
    //Celda celdaParaProbar(0, vector3);
    int i, j;
    while (currentDefense != defenses.end() && maxAttemps > 0) {
        Celda celda = Q.top();
        Q.pop();
        (*currentDefense)->position = celda.vector3;
        if (factibilidad((*currentDefense), obstacles, defensesPlaced, mapWidth, mapHeight)) {
            (*currentDefense)->position = celda.vector3;
            defensesPlaced.push_back((*currentDefense));
            ++currentDefense;
        } else {
            std::cout << "NO SE PUEDE COLOCAR DEFENSA" << std::endl;
        }
    }

#ifdef PRINT_DEFENSE_STRATEGY

    float** cellValues = new float* [nCellsHeight]; 
    for(int i = 0; i < nCellsHeight; ++i) {
       cellValues[i] = new float[nCellsWidth];
       for(int j = 0; j < nCellsWidth; ++j) {
           cellValues[i][j] = ((int)(cellValue(i, j))) % 256;
       }
    }
    dPrintMap("strategy.ppm", nCellsHeight, nCellsWidth, cellHeight, cellWidth, freeCells
                         , cellValues, std::list<Defense*>(), true);

    for(int i = 0; i < nCellsHeight ; ++i)
        delete [] cellValues[i];
    delete [] cellValues;
    cellValues = NULL;

#endif
}
