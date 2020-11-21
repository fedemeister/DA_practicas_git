// ###### Config options ################


// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

using namespace Asedio;

// código basado en https://stackoverflow.com/questions/8617683/return-a-2d-array-from-a-function
std::vector<std::vector<float>> create2DArray(unsigned height, unsigned width) {
    return std::vector<std::vector<float>>(height, std::vector<float>(width + 0, 0));
}


class objetoMochila {
public:
    float valor_;
    unsigned int peso_;
    int id_;

    objetoMochila(float valor, unsigned int peso, int id) : valor_(valor), peso_(peso), id_(id) {}

};

void valueDefenses(const std::list<Defense *> &listaDefensas, std::vector<objetoMochila> &vectorObjetosMochila,
                   const float &mapWidth, const unsigned int &numeroObstaculos) {
    if (numeroObstaculos >= 21 or mapWidth >= 450) {
        for (auto &&defense: listaDefensas) {
            objetoMochila objetoMochila(1 / defense->radio, defense->cost, defense->id);
            vectorObjetosMochila.push_back(objetoMochila);
        }
    } else {
        for (auto &&defense: listaDefensas) {
            objetoMochila objetoMochila(1 / defense->radio + (1.30f * defense->health) + defense->attacksPerSecond,
                                        defense->cost,
                                        defense->id);
            vectorObjetosMochila.push_back(objetoMochila);
        }
    }
}

void DEF_LIB_EXPORTED
selectDefenses(std::list<Defense *> defenses, unsigned int ases, std::list<int> &selectedIDs, float mapWidth,
               float mapHeight, std::list<Object *> obstacles) {

    selectedIDs.push_front(0);
    ases = ases - defenses.front()->cost;
    defenses.pop_front();

    std::vector<objetoMochila> vectorObjetosMochila;
    valueDefenses(defenses, vectorObjetosMochila, mapWidth, obstacles.size());


    float TSR[defenses.size()][ases];
    //std::vector<std::vector<float>> TSR = create2DArray(defenses.size(),ases);

    for (int i = 0; i < defenses.size(); i++) {
        for (int j = 0; j < ases; j++) {
            if (i != 0) {
                TSR[i][j] = j < vectorObjetosMochila[i].peso_ ? TSR[i - 1][j] : std::max(
                        (float) TSR[i - 1][j],
                        TSR[i - 1][j - vectorObjetosMochila[i].peso_] + vectorObjetosMochila[i].valor_

                );
            } else {
                j < vectorObjetosMochila[0].peso_ ? (TSR[0][j] = 0) : (TSR[i][j] = vectorObjetosMochila[0].valor_);
            }
        }
    }


    for (int i = defenses.size() - 1; i > 0; i--) {
        for (int j = ases - 1; j >= 0; j--) {
            if (i != 0) {
                if (TSR[i][j] != TSR[i - 1][j]) {
                    selectedIDs.push_back(vectorObjetosMochila[i].id_);
                    j = j - vectorObjetosMochila[i].peso_ + 1;
                    --i;
                }
            } else if (TSR[i][j] != 0) {
                selectedIDs.push_back(vectorObjetosMochila[i].id_);
                --i;
            }
        }
    }

    //for (auto&& def: selectedIDs){
    //    std::cout << def << std::endl;
    //}

    //objetoMochila aux(9999, 99, 99);
    //std::vector<objetoMochila> vectorObjetosMochilaPrueba1(defenses.size(), aux);

}
