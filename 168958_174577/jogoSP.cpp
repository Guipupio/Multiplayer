#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>
#include "jogoSP.hpp"

using namespace std::chrono;

extern bool JogadorPerdeu;    //Variaveis importadas do arquivo main
extern bool JogadorGanhou;

#define Vel_H 3.75    //Definicoes de velocidades horizontal e vertical
#define Vel_V 3.1
#define MaxMoedas 4   //Numero de moedas presentes no mapa

//mapa do jogo salvo em um vetor x, y
extern std::vector<std::string> mapa;

//Funcao feita para printar strings na tela utilizando echochar para cada caractere
void printChr(int row, int col,char *frase){
  move(row,col);
  int i = 0;
  while (frase[i] != '\0'){
    echochar(frase[i]);
    i++;
  }
}

//Funcao feita para printar strings na tela utilizando echochar para cada caractere
void printStr(int row, int col,std::string frase){
  move(row,col);
  for(char& c : frase) {
    echochar(c);
  }
}

RelevantData::RelevantData() {
};

void RelevantData::serialize(std::string &buffer_out) {
  std::memcpy((void*)buffer_out.c_str(), &(this->posicoes), sizeof(DadosImportantes));
}

void RelevantData::unserialize(std::string buffer_in) {
  std::memcpy(&(this->posicoes), (void*)buffer_in.c_str(), sizeof(DadosImportantes));
}

void RelevantData::update(std::vector<float> newPacMan, int newPosicoes[4][2],  bool newExisteProjetil,  std::vector<float> newProjetil, uint64_t newDeltaT, int numMoedas, int projeteis_restantes, int jogoAtivo, bool* statusBot){

  for (int i = 0; i <2; i++){
    this->posicoes.PacMan[i] = (int) newPacMan[i];
    for (int k = 0; k< 4 ; k++)
      this->posicoes.Bot[k][i] = newPosicoes[k][i];

    this->posicoes.Projetil[i] = (int) newProjetil[i];    
  }
  
  this->posicoes.existeProjetil = newExisteProjetil;
  
  this->posicoes.deltaT = newDeltaT;
  this->posicoes.numMoedas = numMoedas;
  this->posicoes.projeteis_restantes = projeteis_restantes;
  this->posicoes.jogoAtivo = jogoAtivo;

  for(int i = 0; i< 4; i++){
    this->posicoes.statusBot[i] = statusBot[i];
  }
}


//Seta os valores iniciais do corpo
Corpo::Corpo(std::vector<float> velocidade_init, std::vector<float> posicao_init) {
  this->velocidade = velocidade_init;
  this->posicao = posicao_init;
}

//Seta a posicao inicial do bot
Bot::Bot(std::vector< float > init_posicao){
  this->posicao = init_posicao;
}

//Atualiza a posicao e velocidade do bot de acordo com os valores recebidos
void Bot::update(std::vector<float> velocidade_init,std::vector< float > nova_posicao){
  this->velocidade = velocidade_init;
  this->posicao = nova_posicao;
}

//Atualiza a posicao e velocidade do corpo de acordo com os valores recebidos
void Corpo::update(std::vector<float> nova_velocidade_x, std::vector<float> nova_posicao) {
  this->velocidade = nova_velocidade_x;
  this->posicao = nova_posicao;
}

//Retorna a velocidade do corpo
std::vector< float > Corpo::get_velocidade() {
  return this->velocidade;
}

//Retorna a posicao do corpo
std::vector< float >  Corpo::get_posicao() {
  return this->posicao;
}

//Incrementa o numero de moedas q ja foram coletadas pelo player
void Corpo::addMoeda(){
  this->moedas++;
}

//Retorna o numero de moedas que ja foram coletadas pelo player
int Corpo::getMoeda(){
  return this->moedas;
}

int Corpo::getNumProjeteis(){
  return this->num_projeteis;
}

void Corpo::DisparaProjeteis(){
  this->num_projeteis--;
}

//Retorna a velocidade do bot
std::vector< float > Bot::get_velocidade() {
  return this->velocidade;
}

//Retorna a posicao do bot
std::vector< float >  Bot::get_posicao() {
  return this->posicao;
}

//Aloca a lista de corpos
ListaDeCorpos::ListaDeCorpos() {
  this->corpos = new std::vector<Corpo *>(0);
}

//Aloca a lista de bots
ListaDeBots::ListaDeBots() {
  this->bots = new std::vector<Bot *>(0);
}

//Copia cada um dos corpos presentes em uma lista de corpos para outra lista de corpos
void ListaDeCorpos::hard_copy(ListaDeCorpos *ldc) {
  std::vector<Corpo *> *corpos = ldc->get_corpos();

  for (int k=0; k<corpos->size(); k++) {
    Corpo *c = new Corpo( (*corpos)[k]->get_velocidade(),
                          (*corpos)[k]->get_posicao()
                        );
    this->add_corpo(c);
  }
}

//Copia cada um dos corpos presentes em uma lista de bots para outra lista de bots
void ListaDeBots::hard_copy(ListaDeBots *ldb) {
  std::vector<Bot *> *bots = ldb->get_bots();

  for (int k=0; k<bots->size(); k++) {
    Bot *bot = new Bot((*bots)[k]->get_posicao());
    this->add_bot(bot);
  }
}

//Seta quantidade de iterações que o bot selecionado é proibido de se movimentar em determinada direcao
void Bot::set_permissao(int orientacao, int movimentos){
  this->permissao[orientacao] = movimentos;
}

//Retorna a permissao do bot para se movimentar em determinada direcao
int Bot::get_permissao(int orientacao){
  return this->permissao[orientacao];
}

void Bot::set_ultimaCaptura(char command){
  this->chrUltimaCaptura = command;
}

//Retorna o ultimo caractere do bot para se movimentar em determinada direcao
char Bot::get_ultimaCaptura(){
  return this->chrUltimaCaptura;
}

//Caso não seja 0, decrementa a permissao do bota em relação àquela direcao
void Bot::update_permissao(){
  int i;

  for(i=0;i<4;i++){
    if(this->permissao[i] > 0)
      this->permissao[i]--;
  }
}

void Bot::setVidaBot(bool vida){
  this->estouVivo = vida;
}

//Adiciona um corpo à lista de corpos
void ListaDeCorpos::add_corpo(Corpo *c) {
  (this->corpos)->push_back(c);
}

//Adiciona um bot corpo à lista de bots
void ListaDeBots::add_bot(Bot *bot) {
  (this->bots)->push_back(bot);
  bot->setVidaBot(true);
}

//Exlcui o bot selecionado da lista de bots, fazendo assim ele "morrer"


bool Bot::getStatusVida(){
  return this->estouVivo;
}

//Retorna o ponteiro que aponta para os corpos da lista de corpos
std::vector<Corpo*> *ListaDeCorpos::get_corpos() {
  return (this->corpos);
}

//Retorna o ponteiro que aponta para os bots corpos da lista de bots
std::vector<Bot*> *ListaDeBots::get_bots() {
  return (this->bots);
}

//Da os valores iniciais ao projetil
Projetil::Projetil(std::vector< float > velocidade, std::vector< float > posicao, bool tenhoProjetil, bool aindaExisto){
  this->velocidade = velocidade;
  this->posicao = posicao;
  this->tenhoProjetil = tenhoProjetil;
  this->aindaExisto = aindaExisto;
} 

void Projetil::set_sentido(int newSentido){
    this->sentido = newSentido;
}
void Projetil::set_direcao (int newDirecao){
    this->direcao = newDirecao;
}

int Projetil::get_sentido(){
  return this->sentido;
}

int Projetil::get_direcao(){
  return this->direcao;
}

//Retorna a velocidade do projetil
std::vector<float>  Projetil::get_velocidade(){
  return this->velocidade;
}

//Retorna a posicao (x e y) atual do projetil
std::vector<float>  Projetil::get_posicao(){
  return this->posicao;
}

//Retorna a flag que diz se o player ainda possui projeteis para atirar
bool Projetil::get_tenhoProjetil(){
  return this->tenhoProjetil;
}

//Retorna a flag que diz se o projetil ainda existe ou não no mapa
bool Projetil::get_aindaExisto(){
  return this->aindaExisto;
}

//Copia todos os parametros do projetil atual (que é aquele impresso na tela) para o projetil antigo (que é aquele apagado da tela)
void Projetil::hard_copy(Projetil *old_projetil, Projetil *projetil) {
  old_projetil = new Projetil( projetil->get_velocidade(),
                               projetil->get_posicao(),
                               projetil->get_tenhoProjetil(),
                               projetil->get_aindaExisto()
                             );
}

//Atualiza os parametros do projetil
void Projetil::status_projetil(bool tenhoProjetil, bool aindaExisto){
  this->tenhoProjetil = tenhoProjetil;
  this->aindaExisto = aindaExisto;
}

//Valore iniciais passados para a classe fisica
Fisica::Fisica(ListaDeCorpos *ldc, ListaDeBots *ldb, Projetil *prj) {
  this->lista = ldc;
  this->listaBots = ldb;
  this->projetil = prj;
}

//Com base no tempo decorrido e na posicao do player, calcula a nova posicao e velocidade do bot
/**Detalhando a lógica (o mesmo racioncínio é usado para todos os bots):   
  *A perseguição do player feita pelo bot é baseada em distâncias. A lógica consiste em sempre verificar qual a maior distância (horizontal ou vertical)
  *e se movimentar de modo a diminuir a maior das duas. Assim que ele calcula a nova posição x ou y, verifica se tal posição no mapa é um vazio, ou seja, 
  *se não possui uma parede. Caso não possua, ele anda naquela direção. No caso de possuir uma parede, foi implementada uma lógica de permissões, que faz
  *com que o bot não possa mais andar naquela direção por um certo número de iterações, evitando assim que ele volte para o mesmo lugar e se mantenha travado.
  *O numero máximo de iterações foi pensado para ser o suficiente para tirar o bot dos pontos mais críticos do mapa, mas sem trava-lo em todas as direções.
  *Sabendo disso, o bot verifica primeiramente se a maior diŝtância é a vertical. Ele irá se mover nesta direção em duas ocasiões: quando esta condição for
  *realmente satisfeita (e ele tiver permissão para se movimentar naquela direção) ou quando ele estiver impedido de se movimentar na horizontal. 
  *Desse modo, ainda que ele primeiramente tenha que se mover na direção da menor distância, ele não irá ficar travado na tentativa de "atravessar uma parede".
  *Obs1.: apenas para evitar cálculos desnecessários, a lógica foi implementada na ordem inversa da explicada acima, pois ele só calcula a nova posição se tiver 
  *permissão pra se movimentar naquela direção. Entretanto, para fins elucidativos, esta ordem de explicação parece mais fácil.
  *Obs2.: quando a permissão na direção[z] vale 0, significa que ele tem que ficar 0 iterações sem se mover naquela direção (portanto, pode fazer o movimento).
  *Já quando a permissão na direção[z] é maior do que 0, ele não pode fazer aquele movimento por aquele determinado número de iterações.
**/
void Fisica::update_bot(float deltaT, Projetil *projetil, char* ultimosCaracteres){
  int j, aux = 0, sentido = 0, direcao = 0;
  std::vector<Bot *> *bot = this->listaBots->get_bots();
  std::vector<Corpo *> *corpo = this->lista->get_corpos();
  
  for (int i = 0; i < (*bot).size(); i++) {
    if ((*bot)[i]->getStatusVida()){ 
      (*bot)[i]->set_ultimaCaptura(ultimosCaracteres[i]);
      char c = (*bot)[i]->get_ultimaCaptura();

      if (c=='s' or c=='S' or c ==31) {   //Caso em que o jogador deseja descer o player
          sentido = +1;     //sentido positivo (para baixo)
          direcao = 0;      //direcao vertical
        }
        if (c=='w' or c=='W' or c == 30) {  //Caso em que o jogador deseja subir o player
          sentido = -1;     //sentido negativo (para cima)
          direcao = 0;      
        }
        if (c=='a' or c=='A' or c == 17){   //Caso em que o jogador deseja mover o player para a esquerda
          sentido = 0;     //sentido negativo (para para a esquerda)
          direcao = -1;      //direcao horizontal
        }
        if (c=='d' or c=='D' or c == 16){   //Caso em que o jogador deseja mover o player para a direita
          sentido = 0;     //sentido positivo (para direita)
          direcao = 1;
        }

      std::vector< float > new_vel = (*bot)[i]->get_velocidade();
      std::vector< float > new_pos = (*bot)[i]->get_posicao();
      
      new_vel[0] = Vel_H*direcao;
      new_vel[1] = Vel_V*sentido;

      new_pos[0] = (*bot)[i]->get_posicao()[0] + (float)deltaT * new_vel[0]/1000; 
      new_pos[1] = (*bot)[i]->get_posicao()[1] + (float)deltaT * new_vel[1]/1000;  

      if (mapa[(int) new_pos[1]][(int) new_pos[0]] == ' ' and (*bot)[i]->getStatusVida()){   //Verifica se o corpo pode se mover para aquela posição, ou seja, se não existe parede
        (*bot)[i]->update(new_vel, new_pos);
      }


      if(projetil->get_aindaExisto() == true){
        if((int)projetil->get_posicao()[0] == (int)new_pos[0] and (int)projetil->get_posicao()[1] == (int)new_pos[1]){
          (*bot)[i]->setVidaBot(false);
        }
      }

      if((int)(*corpo)[0]->get_posicao()[0] == (int)new_pos[0] and (int)(*corpo)[0]->get_posicao()[1] == (int)new_pos[1]){
          JogadorPerdeu = true;
      }

    }
  }
}

 //Calcula os novos parametros dos corpos com base na direção e sentido definidos pelo jogador 
void Fisica::update_corpo(float deltaT, int sentido, int direcao) {
  std::vector<Corpo *> *c = this->lista->get_corpos();
  for (int i = 0; i < (*c).size(); i++) {

    std::vector< float > new_vel = (*c)[i]->get_velocidade();
    std::vector< float > new_pos = (*c)[i]->get_posicao();
    
    new_vel[0] = Vel_H*sentido;
    new_vel[1] = Vel_V*sentido;

    new_pos[direcao] = (*c)[i]->get_posicao()[direcao] + (float)deltaT * new_vel[direcao]/1000;  

    if (mapa[(int) new_pos[1]][(int) new_pos[0]] == ' '){   //Verifica se o corpo pode se mover para aquela posição, ou seja, se não existe parede
      (*c)[i]->update(new_vel, new_pos);
    }
    if(mapa[(int) new_pos[1]][(int) new_pos[0]] == '$'){    //Verifica se naquela posição há uma moeda para ser coletada
      (*c)[i]->addMoeda();
      if ((*c)[i]->getMoeda() == MaxMoedas){          //Caso o número de moedas tenha atingido o limite, o jogador venceu
        JogadorGanhou = true;
      }

      //Printa na tela o número de moedas já coletadas pelo jogador
      char msg[25];
      sprintf(msg,"Moedas Coletadas: ");
      printChr(23,6,msg);
      for (int num = 0; num < (*c)[i]->getMoeda(); num++)
        echochar('$');

      //Apaga a posição do mapa que possuía a moeda
      mapa[(int) new_pos[1]][(int) new_pos[0]] = ' ';
      (*c)[i]->update(new_vel, new_pos);
    }
  }
}

//Calcula a nova posição do projetil com base no sentido e direção de movimento fornecidos
void Fisica::update_projetil(float deltaT, int sentido, int direcao) {

  // Atualiza parametros do projetil!
  std::vector<float> new_vel = {0,0};
  std::vector<float> new_pos = this->projetil->get_posicao();
       
  new_vel[0] = Vel_H*sentido*1.5;
  new_vel[1] = Vel_V*sentido*1.5;

  new_pos[direcao] = this->projetil->get_posicao()[direcao] + (float)deltaT * new_vel[direcao]/1000; 

  //Verifica se a nova posição é um espaço vazio, pois caso não seja, o projetil "morre"
  if (mapa[(int) new_pos[1]][(int) new_pos[0]] == ' '){
    this->projetil->update_projetil(new_vel, new_pos);
  }else{
    projetil->status_projetil(true, false);
  }
}

//Atualiza os parametros do projetil
void Projetil::update_projetil(std::vector< float > nova_velocidade, std::vector< float > nova_posicao){
  this->velocidade = nova_velocidade;
  this->posicao = nova_posicao;
}

//Inicia os valores da tela, criando uma lista de corpos, bots e projeteis antigos para salvar a última posição deles para que assim possam ser apagadas do mapa
Tela::Tela(ListaDeCorpos *ldc, ListaDeBots *ldb, Projetil *projetil, int maxI, int maxJ, float maxX, float maxY) {
  this->lista = ldc;
  this->listaBots = ldb;
  this->lista_anterior = new ListaDeCorpos();
  this->lista_anterior->hard_copy(this->lista);
  this->listaBots_anterior = new ListaDeBots();
  this->listaBots_anterior->hard_copy(this->listaBots);
  this->maxI = maxI;
  this->maxJ = maxJ;
  this->maxX = maxX;
  this->maxY = maxY;
  this->projetil = projetil;
  this->projetil_old = new Projetil({0,0},{10,10},true,false); 
  this->projetil_old->hard_copy(this->projetil_old, this->projetil);
}

//Inicia a tela
void Tela::init() {
  initscr();             /* Start curses mode     */
  raw();                 /* Line buffering disabled */
  curs_set(0);           /* Do not display cursor */
}

//Recebe todos os dados que precisam ser impressos na tela (corpo, bot, projetil e tempo de jogo)
void Tela::update(Projetil *projetil, uint64_t deltaT) {
  int x,y;

  std::vector<Corpo *> *corpos_old = this->lista_anterior->get_corpos();
  std::vector<Bot *> *bots_old = this->listaBots_anterior->get_bots();
  this->projetil = projetil;

  //Apaga a última posição em que se encontrava o projetil
  x = (int) (projetil_old->get_posicao()[1]) * \
      (this->maxJ / this->maxY);
  y = (int) (projetil_old->get_posicao()[0]) * \
      (this->maxI / this->maxX);
  move(x, y);   /* Move cursor to position */
  echochar(' ');  /* Prints character, advances a position */

  //Caso o projetil ainda exista, printa sua nova posição na tela
  if(projetil->get_tenhoProjetil() == true and projetil->get_aindaExisto() == true){
    x = (int) (projetil->get_posicao()[1]) * \
        (this->maxJ / this->maxY);
    y = (int) (projetil->get_posicao()[0]) * \
        (this->maxI / this->maxX);    
    move(x, y);   /* Move cursor to position */
    echochar('*');  
    //Atualiza os velhos paramentros do projetil para os novos, para que sejam removidos na próxima atualização do projetil
    projetil_old->update_projetil(  
                                    projetil->get_velocidade(),
                                    projetil->get_posicao()
                                  );
  }

  //Apaga corpos na tela
  for (int k=0; k<corpos_old->size(); k++)
  {
    x = (int) ((*corpos_old)[k]->get_posicao()[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((*corpos_old)[k]->get_posicao()[0]) * \
        (this->maxI / this->maxX);
    move(x, y);   /* Move cursor to position */
    echochar(' ');  
  }

  //Apaga bots da tela
  for (int k=0; k<bots_old->size(); k++)
  {
    x = (int) ((*bots_old)[k]->get_posicao()[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((*bots_old)[k]->get_posicao()[0]) * \
        (this->maxI / this->maxX);
    move(x, y);   /* Move cursor to position */
    echochar(' ');  
  }

  std::vector<Corpo *> *corpos = this->lista->get_corpos();
  std::vector<Bot *> *bots = this->listaBots->get_bots();

  for (int k=0; k<corpos->size(); k++)
  {
    x = (int) ((*corpos)[k]->get_posicao()[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((*corpos)[k]->get_posicao()[0]) * \
        (this->maxI / this->maxX);
    
     // Resolve bug de printar fora da tela
    if (x < LINES and y < COLS and x > -1 and y > -1){
      move(x, y);   /* Move cursor to position */
      if(mapa[x][y] == ' '){
        echochar('O');  /* Prints character, advances a position */  
      }
    }

    // Atualiza corpos antigos
    (*corpos_old)[k]->update(  
                                (*corpos)[k]->get_velocidade(),\
                                (*corpos)[k]->get_posicao());
  }

  //Printa os bots na tela
  for (int k=0; k<bots->size(); k++)
  {
    x = (int) ((*bots)[k]->get_posicao()[1]) * \
        (this->maxJ / this->maxY);
    y = (int) ((*bots)[k]->get_posicao()[0]) * \
        (this->maxI / this->maxX);
    
     // Resolve bug de printar fora da tela
    if((*bots)[k]->getStatusVida()){
      if (x < LINES and y < COLS and x > -1 and y > -1){
        move(x, y);   /* Move cursor to position */
        echochar('@');  
      }
    }
    // Atualiza bots antigos
    (*bots_old)[k]->update((*bots)[k]->get_velocidade(),(*bots)[k]->get_posicao());
  }

  //Calcula o tempo de jogo restante e printa na tela
   char TempoRestante[15];
   uint min = deltaT / 60000;
   uint sec = (deltaT)  / 1000 - min * 60;;

  sprintf(TempoRestante, "0");
  sprintf(TempoRestante, "%s%d:",TempoRestante,min);
  // TempoRestante = TempoRestante + ":";
  if (sec > 9)
    sprintf(TempoRestante, "%s%d",TempoRestante,sec);
  else
    sprintf(TempoRestante, "%s0%d",TempoRestante,sec);
  

  if (sec > 0 and (*corpos)[0]->getMoeda() == MaxMoedas)
  {
    JogadorGanhou = true;
  }else if (sec < 1 and min == 0){
    JogadorGanhou = true;
  }else{
   char msg[20];
    sprintf(msg,"Tempo");
    printChr(2,73, msg);
    sprintf(msg,"Restante");
    printChr(3,72, msg);
    printChr(4,73,TempoRestante);
   
  }

   // Atualiza tela
  refresh();
}

void Tela::stop() {
  endwin();
}

//Lê o vetor x, y definido e imprime o mapa na tela
void Tela::geraMapa(){
  std::ifstream file1("screens/Map.txt");
  std::string str;
  while (std::getline(file1, str))
  {
      mapa.push_back(str);  //Copiando mapa
  }

 for (int row = 1; row < mapa.size() - 1; row++){
  for(int col = 1; col<mapa[0].size() - 1; col++){
      move(row,col);
      echochar(mapa[row][col]);
    }
  }

  refresh();

}

Tela::~Tela() {
  this->stop();;
}

void threadfun (char *keybuffer, int *control)
{
  char c;
  while ((*control) == 1) {
    c = getch();
    if (c!=ERR) (*keybuffer) = c;
    else (*keybuffer) = 0;
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }
  return;
}


Teclado::Teclado() {
}

Teclado::~Teclado() {
}

void Teclado::init() {
  // Inicializa ncurses
  raw();                 /* Line buffering disabled */
  keypad(stdscr, TRUE);  /* We get F1, F2 etc..   */
  noecho();              /* Don't echo() while we do getch */
  curs_set(0);           /* Do not display cursor */

  this->rodando = 1;
  std::thread newthread(threadfun, &(this->ultima_captura), &(this->rodando));
  (this->kb_thread).swap(newthread);

}

void Teclado::stop() {
  this->rodando = 0;
  (this->kb_thread).join();
}

char Teclado::getchar() {
  char c = this->ultima_captura;
  this->ultima_captura = 0;
  return c;
}