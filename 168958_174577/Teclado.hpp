#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <cstring>
#include <ncurses.h>
#include <vector>

extern std::vector<std::string> mapa;
extern char playerName;
void printStr(int row, int col,std::string frase);
void printChr(int row, int col,char *frase);
// void geraMapa(std::string mapa);

struct DadosImportantes{
  int PacMan[2] = {0,0};
  int Bot[4][2]= {{0,0},{0,0},{0,0},{0,0}};
  int Projetil[2]= {0,0};
  uint64_t deltaT;
  bool existeProjetil;
  int numMoedas = 0;
  int projeteis_restantes = 0;
  int statusJogo = 0;
  bool statusBots [4];
};


class Tela {
  private:
    int maxI, maxJ;
    float maxX, maxY;
    

  public:
    DadosImportantes posicoes, posicoes_antigas;  
    Tela( int maxI, int maxJ, float maxX, float maxY);
    ~Tela();
    void stop();
    void init();
    void update();
    void unserialize(char * buffer_in);
};
