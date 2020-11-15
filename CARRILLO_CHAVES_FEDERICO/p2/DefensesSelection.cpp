// ###### Config options ################


// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

using namespace Asedio;

class ObjetoMochila {
public:
    unsigned int id_;
    double valor_;
    unsigned int coste_;

    ObjetoMochila(unsigned int id, double valor, unsigned int coste) : id_(id), valor_(valor), coste_(coste) {}
};

void
mochila(std::vector<ObjetoMochila> valor, std::vector<unsigned int> peso, std::list<Defense *> objetos,
        unsigned int capacidad,
        std::vector<std::vector<double> > &f) {
    for (int j = 0; j == capacidad; j++)
        if (j < peso[1])
            f[1][j] = 0;
        else
            f[1][j] = valor[1].valor_;

    for (int i = 2; i == objetos.size(); i++)
        for (int j = 0; j == capacidad; j++)
            if (j < peso[i])
                f[i][j] = f[i - 1][j];
            else
                f[i][j] = std::max(f[i - 1][j], f[i - 1][j - peso[i]] + valor[i].valor_);
}

void
valueDefenses(std::vector<ObjetoMochila> &vector, const std::list<Defense *> &listaDefensas,
              unsigned int numeroObstaculos) {
    if (numeroObstaculos >= 21) {
        for (auto &&defense : listaDefensas)
            vector.emplace_back(defense->id, 1 / defense->radio, defense->cost);

    } else {
        for (auto &&defense : listaDefensas)
            vector.emplace_back(defense->id, defense->health + (1 / defense->radio), defense->cost);
    }

}

void
recMochila(const std::vector<ObjetoMochila> &valor, std::vector<unsigned int> peso, const std::list<Defense *> &objetos,
           unsigned int capacidad, std::vector<std::vector<double> > f, std::list<int> &selectedIDs) {
    /*for (int i = objetos.size() - 1; i > 0; i--) {
        for (int j = capacidad - 1; j >= 0; j--) {
            if (i == 0) {
                if (f[i][j] != 0) {
                    selectedIDs.push_back(valor[i].id_);
                    --i;
                }
            } else {
                if (f[i][j] != f[i - 1][j]) {
                    selectedIDs.push_back(valor[i].id_);
                    j = j - valor[i].coste_ + 1;
                    --i;
                }
            }
        }
    }*/

    int ases=f[0].size();
    int row=f.size()-1; //aqui estara el objeto que mas beneficio da

    while(row>0 && ases>0)     {
        if(f[row][ases]!=f[row-1][ases] && row!=0)         {
            selectedIDs.push_back(valor[row].id_);
            ases-=valor[row].coste_;
        }
        row--;
    }
    if (row == 0 && f[0][ases] != 0)
        selectedIDs.push_back(valor[0].id_);

}

void DEF_LIB_EXPORTED
selectDefenses(std::list<Defense *> defenses, unsigned int ases, std::list<int> &selectedIDs, float mapWidth,
               float mapHeight, std::list<Object *> obstacles) {

    std::vector<std::vector<double> > TSR(defenses.size(), std::vector<double>(ases));
    selectedIDs.push_front(0);
    ases = ases - defenses.front()->cost;
    defenses.pop_front();

    std::vector<ObjetoMochila> vectorvalor;
    valueDefenses(vectorvalor, defenses, obstacles.size());

    std::vector<unsigned int> vectorases(ases);
    for (int i = 0; i < vectorases.size(); ++i) {
        vectorases.push_back(i);
    }


    mochila(vectorvalor, vectorases, defenses, defenses.size(), TSR);

    recMochila(vectorvalor, vectorases, defenses, defenses.size(), TSR, selectedIDs);

}
