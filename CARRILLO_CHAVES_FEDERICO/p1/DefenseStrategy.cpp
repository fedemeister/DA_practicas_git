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
        int valor_;
        Vector3 vector3_;
        Celda(int valor, const Vector3 &vector3) : valor_(valor), vector3_(vector3) {}
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

int cellValue(int row, int col, bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
              std::list<Object *> obstacles, std::list<Defense *> defenses, bool esCEM) {
    // implemente aqui la función que asigna valores a las celdas
    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;
    //Vector3 centro = cellCenterToPosition(nCellsWidth / 2, nCellsHeight / 2, cellWidth, cellHeight);
    //Vector3 current = cellCenterToPosition(row,col,cellWidth,cellHeight);
    if (esCEM == true) {
		Vector3 mapCenter = Vector3(mapWidth / 2,mapHeight / 2,0);
		Vector3 current = cellCenterToPosition(row,col,cellWidth,cellHeight);
		return abs(_distance(current,mapCenter));
        //return (nCellsWidth - col) * (nCellsHeight - row);
        //return (((nCellsWidth - col/2))/(mapWidth / 2) * (nCellsHeight - row/2)/(mapHeight / 2));
    }
    else {
        List < Defense * >:: const_iterator centroExtraccion = defenses.begin ();
        Vector3 dst;
        int i_out,j_out;
        positionToCell((*centroExtraccion)->position,i_out,j_out,cellWidth,cellHeight);
        dst = cellCenterToPosition(row - i_out, col - j_out, cellWidth, cellHeight);
        return std::max( mapWidth , mapHeight ) - dst.length();
    }

    //return abs(_distance(current,centro));
}

bool factibilidad( float mapWidth,float mapHeight, Defense *defense, std::list<Object *> obstacles, std::vector<Defense *> defensesPlaced) {
    bool factible = true;
    auto currentObstacle = obstacles.begin();
    auto currentDefense = defensesPlaced.begin();

    // Dentro de los bordes del mapa
    if (defense->position.x + defense->radio <= mapWidth && (defense->position.x - defense->radio) >= 0 &&
        (defense->position.y + defense->radio) <= mapHeight && (defense->position.y - defense->radio) >= 0) {}
    else {
        factible = false;
    }

    //No colisiona con defensas ya almacenadas
    while (currentDefense != defensesPlaced.end() && factible) {
        if ((_distance(defense->position, (*currentDefense)->position)
             - (defense->radio + (*currentDefense)->radio)) >= 0) {}
        else {
            factible = false;
        }
        currentDefense++;
    }

    // No colisiona con ningún obstáculo
    while (currentObstacle != obstacles.end() && factible) {
        if ((_distance(defense->position, (*currentObstacle)->position) -
             (defense->radio + (*currentObstacle)->radio)) >= 0) {}
        else {
            factible = false;
        }
        currentObstacle++;
    }

    return factible;
}


void DEF_LIB_EXPORTED
placeDefenses(bool **freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight,
              std::list<Object *> obstacles, std::list<Defense *> defenses) {

    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;

    std::priority_queue<Celda, std::vector<Celda>, ComparaValor> Q; // cola de prioridad para las celdas candidatas

    //std::cout << "******************************************************" << std::endl;
    //std::cout << "freeCells:" << *freeCells << std::endl;
    //std::cout << "nCellsWidth:" << nCellsWidth << std::endl;
    //std::cout << "nCellsHeight:" << nCellsHeight << std::endl;
    //std::cout << "mapWidth:" << mapWidth << std::endl;
    //std::cout << "mapHeight:" << mapHeight << std::endl;
    //std::cout << "******************************************************" << std::endl;


    Vector3 aux_vector3;
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux = Celda(
                    cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, true),
                    aux_vector3);
            Q.push(aux);
        }
    }

    std::vector<Defense *> defensesPlaced;
    //defensesPlaced.sort(ComparaRadioDefensas);
    //defenses.sort([](const Defense *a, const Defense *b) { return a->radio > b->radio; });
    //defenses.sort([](const Defense *a, const Defense *b) { return a->damage > b->damage; });
    auto defensaCandidata = defenses.begin();
    bool CEM_colocado = false;
    // CEM
    while (defensaCandidata != defenses.end() && CEM_colocado == false) {
        Celda celda = Q.top();
        Q.pop();
        (*defensaCandidata)->position = celda.vector3_;
        if (factibilidad(mapWidth, mapHeight, (*defensaCandidata), obstacles, defensesPlaced)) {
            defensesPlaced.push_back((*defensaCandidata));
            ++defensaCandidata;
            CEM_colocado = true;
        }
    }

    // demás defensas
    std::priority_queue<Celda, std::vector<Celda>, ComparaValor2> Q2;   // reseteamos la cola
    for (int i = 0; i < nCellsHeight; i++) {
        for (int j = 0; j < nCellsWidth; j++) {
            aux_vector3 = cellCenterToPosition(i, j, cellWidth, cellHeight);
            Celda aux = Celda(
                    cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses, false),
                    aux_vector3);
            Q2.push(aux);
        }
    }
    //defenses.sort([](const Defense *a, const Defense *b) { return a->health < b->heath; });

    while (defensaCandidata != defenses.end()) {
        Celda celda = Q2.top();
        Q2.pop();
        (*defensaCandidata)->position = celda.vector3_;
        if (factibilidad(mapWidth, mapHeight, (*defensaCandidata), obstacles, defensesPlaced)) {
            defensesPlaced.push_back((*defensaCandidata));
            ++defensaCandidata;
        }
    }     

#ifdef PRINT_DEFENSE_STRATEGY

    float** cellValues = new float* [nCellsHeight]; 
    for(int i = 0; i < nCellsHeight; ++i) {
       cellValues[i] = new float[nCellsWidth];
       for(int j = 0; j < nCellsWidth; ++j) {
           cellValues[i][j] = ((int)(cellValue(i, j, freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses,  false))) % 256;
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
