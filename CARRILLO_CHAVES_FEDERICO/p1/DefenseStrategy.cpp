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

using namespace Asedio;

// Estructura que representa una celda candidata
// valor - valor de la celda
// coordenadas - coordenadas de la celda en Vector3
// Celda(int valor, Vector3 vector3) - Constructor
struct Celda {
    int valor;
	Vector3 coordenada;
	Celda(int valor, Vector3 vector3) : valor(valor),coordenada(vector3){}
};

List<Celda> candidatos;


// Devuelve la posición en el mapa del centro de la celda (i,j)
// i - fila
// j - columna
// cellWidth - ancho de las celdas
// cellHeight - alto de las celdas
Vector3 cellCenterToPosition(int i, int j, float cellWidth, float cellHeight){ return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0); }

// Devuelve la celda a la que corresponde una posición en el mapa
// pos - posición que se quiere convertir
// i_out - fila a la que corresponde la posición pos (resultado)
// j_out - columna a la que corresponde la posición pos (resultado)
// cellWidth - ancho de las celdas
// cellHeight - alto de las celdas
void positionToCell(const Vector3 pos, int &i_out, int &j_out, float cellWidth, float cellHeight){ i_out = (int)(pos.y * 1.0f/cellHeight); j_out = (int)(pos.x * 1.0f/cellWidth); }

bool posicionDentroMapa(Celda& prometedora,float radio,float mapWidth, float mapHeight){
	if(prometedora.coordenada.x + radio >= mapWidth || prometedora.coordenada.y + radio >= mapHeight
	   || prometedora.coordenada.x - radio <= 0.0 || prometedora.coordenada.y - radio <= 0.0)
	return false;
    else
    {
        return true;
    }
}

float cellValue(int row, int col, bool** freeCells, int nCellsWidth, int nCellsHeight
	, float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses) {
	return 0; // implemente aqui la función que asigna valores a las celdas
}

bool factibilidad (Defense* currentDefense, std::list<Defense*> defenses, std::list<Object*> obstacles, Celda prometedora, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight) {
    //Que no se salga del mapa
    //Distancia Euclídea con otras defensas. - Esto solo para cuando ya se haya colocado el C.E.M
    //Tener en cuenta los obstáculos
    //si celda_prometedora.posición es posición válida Y celda_prometedora.posición!=obstáculo_cualquiera.posición
	//	devolver VERDADERO
    if(posicionDentroMapa(prometedora,currentDefense->radio,mapWidth,mapHeight)) {
        return true;
    }
    else {
        return false;
    }
    
}

void DEF_LIB_EXPORTED placeDefenses(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight
              , std::list<Object*> obstacles, std::list<Defense*> defenses) {

    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight; 

    int maxAttemps = 1000;
    List<Defense*>::iterator currentDefense = defenses.begin();
    std::cout << "******************************************************" << std::endl;
    std::cout << "freeCells:" << freeCells << std::endl;
    std::cout << "nCellsWidth:" << nCellsWidth << std::endl;
    std::cout << "nCellsHeight:" << nCellsHeight << std::endl;
    std::cout << "mapWidth:" << mapWidth << std::endl;
    std::cout << "mapHeight:" << mapHeight << std::endl;  
    std::cout << "******************************************************" << std::endl;
    int i=0;
    while(currentDefense != defenses.end() && maxAttemps > 0) {
        

        //(*currentDefense)->position.x = ((int)(_RAND2(nCellsWidth))) * cellWidth + cellWidth * 0.5f;
        //(*currentDefense)->position.y = ((int)(_RAND2(nCellsHeight))) * cellHeight + cellHeight * 0.5f;
        if (i==0){
            //(*currentDefense)->position.x = 0 * cellWidth  + cellWidth * 0.5f;
            //(*currentDefense)->position.y = 3 * cellHeight + cellHeight * 0.5f;
            //(*currentDefense)->position.z = 0.0;
            (*currentDefense)->position = cellCenterToPosition(0,0,cellWidth,cellHeight);
        }
        else {
            (*currentDefense)->position.x = 0 * cellWidth + cellWidth * 0.5f;
            (*currentDefense)->position.y = 0 * cellHeight + cellHeight * 0.5f;
            (*currentDefense)->position.z = 0.0;
        }        

        ++currentDefense;
        i++;
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
