#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <string.h>
#include <cstring>

#include "Teclado.hpp"

#define MaxMoedas 4

using namespace std::chrono;

extern std::vector<std::string> mapa;
extern char playerName;

//Funcao feita para printar strings na tela utilizando echochar para cada caractere
void printStr(int row, int col,std::string frase){
  move(row,col);
  for(char& c : frase) {
    echochar(c);
  }
}

void printChr(int row, int col,char *frase){
  move(row,col);
  int i = 0;
  while (frase[i] != '\0'){
    echochar(frase[i]);
    i++;
  }
  
}
// void geraMapa(char *mapa[73]){
// for (int row = 1; row < 23 - 1; row++){
//   for(int col = 1; col<72 - 1; col++){
//       move(row,col);
//       echochar(mapa[row][col]);
//     }
//   }
// }

Tela::Tela(int maxI, int maxJ, float maxX, float maxY) {
  this->maxI = maxI;
  this->maxJ = maxJ;
  this->maxX = maxX;
  this->maxY = maxY;
}

void Tela::init() {
  initscr();             /* Start curses mode     */
  raw();                 /* Line buffering disabled */
  curs_set(0);           /* Do not display cursor */
}




void Tela::update(/*Projetil *projetil, uint64_t this->posicoes.deltaT*/) {
  int x,y;

  int corpos[2];
  int corpos_old[2]; 
  int bots_old[4][2];
  int bots[4][2];
  int projetil[2];
  int projetil_old[2];

  //this->posicoes_antigas.Projetil = this->posicoes_antigas.Projetil;
  for (int i = 0; i< 2 ;i++){
    corpos[i] = this->posicoes.PacMan[i];
    corpos_old[i] = this->posicoes_antigas.PacMan[i];
    for (int j = 0; j < 4; j++){      
      bots[j][i] = this->posicoes.Bot[j][i];
      bots_old[j][i] = this->posicoes_antigas.Bot[j][i];   
    }
    
    projetil[i] = this->posicoes.Projetil[i];
    projetil_old[i] = this->posicoes_antigas.Projetil[i];
  }


  //Apaga a última posição em que se encontrava o projetil

  x = (int) (projetil_old[1]) * \
      (this->maxJ / this->maxY);
  y = (int) (projetil_old[0]) * \
      (this->maxI / this->maxX);

  move(x, y);   /* Move cursor to position */
  echochar(' ');  /* Prints character, advances a position */
  
  //Caso o projetil ainda exista, printa sua nova posição na tela
  if(this->posicoes.existeProjetil == true){
    x = (int) (projetil[1]) * \
        (this->maxJ / this->maxY);
    y = (int) (projetil[0]) * \
        (this->maxI / this->maxX);    
    move(x, y);   /* Move cursor to position */
    echochar('*');  
    //Atualiza os velhos paramentros do projetil para os novos, para que sejam removidos na próxima atualização do projetil
    for (int i = 0; i < 2 ; i++)
      this->posicoes_antigas.Projetil[i] = projetil[i];
  }

  //Apaga corpos na tela
  // for (int k=0; k<corpos_old.size(); k++)
  // {
    x = (int) ((corpos_old)[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((corpos_old)[0]) * \
        (this->maxI / this->maxX);
    move(x, y);   /* Move cursor to position */
    echochar(' ');  
  // }

  //Apaga bots da tela
  for (int k=0; k<4; k++)
  {
    x = (int) (bots_old[k][1]) * \
        (this->maxJ / this->maxY);
    y = (int) (bots_old[k][0]) * \
        (this->maxI / this->maxX);
    move(x, y);   /* Move cursor to position */
    echochar(' ');  
  }
  // corpos->size
  for (int k=0; k<1; k++)
  {
    x = (int) ((corpos)[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((corpos)[0]) * \
        (this->maxI / this->maxX);
    
     // Resolve bug de printar fora da tela
    if (x < LINES and y < COLS and x > -1 and y > -1){
      move(x, y);   /* Move cursor to position */
      if(mapa[x][y] == ' '){
        echochar('O');  /* Prints character, advances a position */  
      }
      if(mapa[x][y] == '$'){
        mapa[x][y] = ' ';
        echochar('O');
      }
    }
    // Atualiza corpos antigos
    for (int i=0; i<2; i++)
      this->posicoes_antigas.PacMan[i] = corpos[i];
  }
  //Printa os bots na tela
  // for (int k=0; k<bots.size(); k++)

  //////////////////////////////////////////////////////////////////////////////////////////////
  for (int k=0; k<4; k++)
  {
    x = (int) (bots[k][1]) * \
        (this->maxJ / this->maxY);
    y = (int) (bots[k][0]) * \
        (this->maxI / this->maxX);
    
     // Resolve bug de printar fora da tela
    if (playerName == k + 48){
      if (this->posicoes.statusBots[k] and x < LINES and y < COLS and x > -1 and y > -1){
        move(x, y);   /* Move cursor to position */
        echochar('0');  
      }  
    }
    else{
      if (this->posicoes.statusBots[k] and x < LINES and y < COLS and x > -1 and y > -1){
        move(x, y);   /* Move cursor to position */
        echochar('@');  
      }
    }
    

    // Atualiza bots antigos
    for (int i=0; i<2; i++){
      this->posicoes_antigas.Bot[k][i] = bots[k][i];
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////

//Calcula o tempo de jogo restante e printa na tela

  char TempoRestante[15];
  int min = this->posicoes.deltaT / 60000;
  int sec = (this->posicoes.deltaT) / 1000 - min * 60;
  



  sprintf(TempoRestante, "0");
  sprintf(TempoRestante, "%s%d:",TempoRestante,min);
  // TempoRestante = TempoRestante + ":";
  if (sec > 9)
    sprintf(TempoRestante, "%s%d  ",TempoRestante,sec);
  else
    sprintf(TempoRestante, "%s0%d  ",TempoRestante,sec);

    char msg[20];
    sprintf(msg,"Tempo");
    printChr(2,73, msg);
    sprintf(msg,"Restante");
    printChr(3,72, msg);
    printChr(4,73,TempoRestante);
    

   // Atualiza tela
  refresh();
}

void Tela::stop() {
  endwin();
}

Tela::~Tela() {
  this->stop();;
}


void Tela::unserialize(char * buffer_in) {

  std::memcpy(&(this->posicoes), (void*)buffer_in, sizeof(DadosImportantes));
}
