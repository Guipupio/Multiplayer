#include <thread>
#include <string>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern bool JogadorPerdeu;
extern bool JogadorGanhou; 
extern std::vector<std::string> mapa;

void printChr(int row, int col,char *frase);
void printStr(int row, int col,std::string frase);

struct DadosImportantes{
  int PacMan[2] = {0,0};
  int Bot[4][2]= {{0,0},{0,0},{0,0},{0,0}};
  int Projetil[2]= {0,0};
  uint64_t deltaT;
  bool existeProjetil;
  int numMoedas = 0;
  int projeteis_restantes = 0;
  int jogoAtivo = 1;
  bool statusBot[4];
};



class Corpo {
  private:
  std::vector<float> velocidade ={0,0};
  std::vector<float> posicao = {0,0};
  int moedas = 0;
  int num_projeteis = 3;      //Numero inicial de projeteis do player

  public:
  Corpo(std::vector< float > velocidade_x, std::vector< float > posicao);
  void update(std::vector<float> nova_velocidade,std::vector<float> nova_posicao);
  std::vector<float> get_velocidade();
  std::vector<float> get_posicao();
  void addMoeda();
  int getMoeda();
  int getNumProjeteis();
  void DisparaProjeteis();
};


class Bot {
  private:  
  std::vector<float> velocidade ={0,0};
  std::vector<float> posicao = {0,0}; 
  std::vector<int> permissao = {0,0,0,0}; //Salva as permissões de cada direção, sendo elas respectivamente: cima, baixo, direita, esquerda
  char chrUltimaCaptura;
  bool estouVivo = false;

  public:
  Bot(std::vector< float > posicao);
  void update(std::vector<float> nova_velocidade,std::vector<float> nova_posicao);
  std::vector<float> get_velocidade();
  std::vector<float> get_posicao();

  void update_permissao();      //Decrementa 1 de cada direção que possui permissão maior que 0
  void set_permissao(int orientacao, int movimentos);
  char get_ultimaCaptura();
  void set_ultimaCaptura(char command);
  int get_permissao(int orientacao);
  bool getStatusVida();
  // void kill_bot();       //Retira da lista de bots o bot da posição indicada, "matando" assim o bot
  void setVidaBot(bool vida);
};

class RelevantData {
  private:
    DadosImportantes posicoes;
  
  public:
    RelevantData();
    void serialize(std::string &buffer_out);
    void update(std::vector<float> newPacMan,  int newPosicoes[4][2],  bool newExisteProjetil,  std::vector<float> newProjetil, uint64_t deltaT, int numMoedas, int projeteis_restantes, int jogoAtivo, bool* statusBot);
    void unserialize(std::string buffer_in);
    
};

class Projetil {
  private:
  std::vector<float> velocidade ={0,0};
  std::vector<float> posicao = {0,0};
  bool tenhoProjetil;   //Flag que indica se o player ainda possui projeteis para serem lançados
  bool aindaExisto;     //Flag que indica se o projetil ainda está sendo printado na tela
  int sentido = 0, direcao = 0; //Variaveis para salvar o sentido e a direcao do projetil
  
  public:
  Projetil(std::vector< float > velocidade, std::vector< float > posicao, bool tenhoProjetil, bool aindaExisto);
  void update_projetil(std::vector< float > nova_velocidade, std::vector< float > nova_posicao);
  void status_projetil(bool tenhoProjetil, bool aindaExisto);     //Altera o status do projetil: se ainda existe ou não na tela e se o player ainda possui projeteis 
  std::vector<float>  get_velocidade();
  std::vector<float>  get_posicao();
  void hard_copy(Projetil * old_projetil, Projetil *projetil);    //Copia para um projetil velho os paramêtros do projetil atual, para que assim a ultima posição dele seja apagada
  bool get_tenhoProjetil();
  bool get_aindaExisto();
  int get_sentido();
  int get_direcao();
  void set_sentido(int newSentido);
  void set_direcao (int newDirecao);
};

class ListaDeCorpos {
 private:
    std::vector<Corpo*> *corpos;

  public:
    ListaDeCorpos();
    void hard_copy(ListaDeCorpos *ldc);    //Copia para um corpo velho os paramêtros do corpo atual, para que assim a ultima posição dele seja apagada
    void add_corpo(Corpo *c);              //Adiciona um corpo à lista de corpo
    std::vector<Corpo*> *get_corpos();     //Retorna o ponteiro da lista de corpos
};

class ListaDeBots {
 private:
    std::vector<Bot*> *bots;

  public:
    ListaDeBots();
    void hard_copy(ListaDeBots *ldc); //Copia para um bot velho os paramêtros do bot atual, para que assim a ultima posição dele seja apagada
    void add_bot(Bot *bot);           //Adiciona um bot à lista de bots
    // void kill_bot(int posicao);       //Retira da lista de bots o bot da posição indicada, "matando" assim o bot
    std::vector<Bot*> *get_bots();    //Retorna o ponteiro da lista de bots
};

class Fisica {
  private:
    ListaDeCorpos *lista;
    ListaDeBots *listaBots;
    Projetil  *projetil;

  public:
    Fisica(ListaDeCorpos *ldc, ListaDeBots *listaBots, Projetil *prj);
    void update_corpo(float deltaT, int sentido, int direcao);      //Calcula a nova posição e velocidade do corpo
    void update_projetil(float deltaT, int sentido, int direcao);   //Calcula a nova posição e velocidade do projetil
    void update_bot(float deltaT, Projetil *projetil, char* ultimoComandoFantasma);              //Calcula a nova posição e velocidade de cada bot
};

class Tela {
  private:
    ListaDeCorpos *lista, *lista_anterior;
    ListaDeBots *listaBots, *listaBots_anterior;
    Projetil *projetil, *projetil_old;
    int maxI, maxJ;
    float maxX, maxY;

  public:
    Tela(ListaDeCorpos *ldc, ListaDeBots *ldb, Projetil *projetil_old, int maxI, int maxJ, float maxX, float maxY);
    ~Tela();
    void geraMapa();
    void stop();
    void init();
    void update(Projetil *projetil, uint64_t deltaT);
};
void threadfun (char *keybuffer, int *control);

class Teclado {
  private:
    char ultima_captura;
    int rodando;

    std::thread kb_thread;

  public:
    Teclado();
    ~Teclado();
    void stop();
    void init();
    char getchar();
};