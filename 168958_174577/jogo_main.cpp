#include "Playback.hpp"
#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "jogoSP.hpp"

#define MAX_CONEXOES 2

#define GameTimeMs 80000 		//Define o tempo máximo de duração do jogo

 bool JogadorPerdeu;			//Variáveis de controle para verificar
 bool JogadorGanhou; 			//  se o jogador venceu ou perdeu
 bool todosJogadoresConectados = false;
 
 std::vector<std::string> mapa;

using namespace std::chrono;
uint64_t get_now_ms() {
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

struct sockaddr_in myself, client;

socklen_t client_size;
int socket_fd;
int connection_fd[MAX_CONEXOES];
int conexao_usada[MAX_CONEXOES];
int running = 1;
char ultimoComandoFantasma[4];


int remover_conexao(int user) {
  if (conexao_usada[user]==1) {
  conexao_usada[user] = 0;
  close(connection_fd[user]);
  }
  return 1;
}


int adicionar_conexao(int new_connection_fd) {
  int i;
  char mensagem[4];
  for (i=0; i<MAX_CONEXOES; i++) {

    if (conexao_usada[i] == 0) {
      conexao_usada[i] = 1;
      connection_fd[i] = new_connection_fd;
       if (i ==  MAX_CONEXOES - 1){
        todosJogadoresConectados = true;       
        for (int user_iterator=0; user_iterator<MAX_CONEXOES; user_iterator++){
          if (conexao_usada[user_iterator] == 1){
            sprintf(mensagem,"OK%d",user_iterator); 
            if (send(connection_fd[user_iterator], mensagem, 4, MSG_NOSIGNAL) == -1)  
              remover_conexao(user_iterator);          
          }
        }
       }                    
      return i;
    }
  }
  return -1;
}

void *wait_connections(void *parameters) {
  int conn_fd;
  int user_id;
  while(running) {
    conn_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
    user_id = adicionar_conexao(conn_fd);
    if (user_id != -1) {
      
    } else {
      // printStr(23,6,"Maximo de usuarios atingido!");
    }
  }
  return NULL;
}

void *receiveKeybord(void *parameters){
  int msglen;
  int user_iterator;
  char input_buffer[2];

  while (running) {
    for (user_iterator=0; user_iterator<MAX_CONEXOES; user_iterator++) {
      if (conexao_usada[user_iterator] == 1) {
        msglen = recv(connection_fd[user_iterator], input_buffer, 2, MSG_DONTWAIT);
        if (msglen > 0) {
          input_buffer[1] = '\0';
          ultimoComandoFantasma[user_iterator] = input_buffer[0];
          
        }
      }
    }
  }

  return NULL;
}

int main ()
{

  uint64_t t0;
  uint64_t t1;
  uint64_t deltaT;
  uint64_t T;
  uint64_t tPause = 0;
  uint64_t tPeteleco;
  uint64_t tFundo = 0;

  pthread_t esperar_conexoes,recebeTeclado;
  int msglen;
  int user_iterator;
  int posicoesAtuais[4][2];
  char output_buffer[60];
  char input_buffer[50];

  float intensities[2] = {1.0,1.0};


  bool statusBots[4];
  bool ativaMusicaProjeto;

  JogadorPerdeu = false;
  JogadorGanhou = false;
  ativaMusicaProjeto = false;
    /* socket, bind, listen */
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  printf("Socket criado\n");
  myself.sin_family = AF_INET;
  myself.sin_port = htons(3001);
  inet_aton("127.0.0.1", &(myself.sin_addr));
  printf("Tentando abrir porta 3001\n");
  if (bind(socket_fd, (struct sockaddr*)&myself, sizeof(myself)) != 0) {
    printf("Problemas ao abrir porta\n");
    return 0;
  }
  printf("Abri porta 3001!\n");
  listen(socket_fd, 2);
  printf("Estou ouvindo na porta 3001!\n");

    /* Dispara thread para ouvir conexoes */
  pthread_create(&esperar_conexoes, NULL, wait_connections, NULL);

  std::string text;
  int i = 0;
  
  int sentido = 0, direcao = 0;		//Varíaveis para salvar o sentido e direção do player de acordo com as teclas w, a, s, d

  bool jogoRodando = false;

  //Sequencia de samples dos audios utilizados no jogo sendo carregados
  Audio::Sample *sampleProjetil;
  sampleProjetil = new Audio::Sample("assets/fire.wav");
  // sampleProjetil->load("assets/fire.dat");

  Audio::Sample *sampleDerrota;
  sampleDerrota = new Audio::Sample("assets/derrota.wav");
  // sampleDerrota->load("assets/derrota.wav");

  Audio::Sample *sampleVitoria;
  sampleVitoria = new Audio::Sample("assets/vitoria.wav");
  // sampleVitoria->load("assets/vitoria.wav");

  Audio::Sample *sampleFundo;
  sampleFundo = new Audio::Sample("assets/fundo.wav");
  // sampleFundo->load("assets/fundo.wav");

  Audio::Player *player;
  player = new Audio::Player();


  // Espera
  while (1) {
    std::this_thread::sleep_for (std::chrono::milliseconds(1));
    t1 = get_now_ms();
    if (t1-t0 > 6500) break;
  }

  //Criacao no player e dos quatro bots, tendo cada um sua posicao inicial definida
  Corpo *pacMan = new Corpo({0,0},{20,11});
  Bot *bot[4]= {new Bot({2,2}),new Bot({65,2}),new Bot({65,20}), new Bot({2,20})};



  RelevantData *positionData = new RelevantData();

  //Cria tanto a lista de corpos quanto a de bots e salva, respectivamente, o player e os quatro bots existentes
  ListaDeCorpos *l = new ListaDeCorpos();
  ListaDeBots *b = new ListaDeBots();
  l->add_corpo(pacMan);
  for (int k = 0; k < 3 ; k++)
    b->add_bot(bot[k]);
  
  // -------------------------

  //Criando as variáveis das classes de projetil, fisica e tela e setando seus valores iniciais
  Projetil *projetil = new Projetil({0,0},{20,11},true,false);
  Fisica *f = new Fisica(l,b,projetil);
  Tela *tela = new Tela(l, b, projetil, 40, 40, 40, 40);

  tela->init();

  //Apos iniciar a tela, imprime o nome do jogo, presente no arquivo telaInit.txt
  std::ifstream file("screens/telaInit.txt");
    std::string str;
    int intCount = 3; 
    while (std::getline(file, str))
    {
        printStr(intCount,5,str);
        intCount++;
    }

  Teclado *teclado = new Teclado();
  teclado->init();

  // uint64_t lensampleFundo = static_cast<float>(sampleFundo->get_data().size());
  // uint64_t lensampleProjetil = static_cast<float>(sampleProjetil->get_data().size());

  //Aguarda Conexao de outros jogadores
  while(true){
    if (todosJogadoresConectados) break;
  }

  // player->init();

  // Thread para leitura do teclado
  pthread_create(&recebeTeclado, NULL, receiveKeybord, NULL);

  //Carrega a contagem regressiva para o inicio do jogo atraves dos arquivos com cada numero e encerra carregando o arquivo de start
  for(int i=5;i>0;i--){
    
    std::ifstream file2("screens/clean.txt");
    intCount = 8; 
    while (std::getline(file2, str))
      {
        printStr(intCount,5,str);
        intCount++;
      } 
    std::ifstream file("screens/" +std::to_string(i)+".txt");
    std::string str;
    int intCount = 8; 
    while (std::getline(file, str))
      {
        printStr(intCount,30,str);
        intCount++;
      }
    std::this_thread::sleep_for (std::chrono::milliseconds(1000));
      
    }

  std::ifstream file2("screens/clean.txt");
  intCount = 8; 
  while (std::getline(file2, str))
    {
      printStr(intCount,5,str);
      intCount++;
    } 
  
  std::ifstream file3("screens/start.txt");
  intCount = 8; 
  while (std::getline(file3, str))
  {
      printStr(intCount,22,str);
      intCount++;
  }
  file3.close();
  std::this_thread::sleep_for (std::chrono::milliseconds(1000));

  //Realiza a impressao do mapa na rela, assim como as indicaçoes de moedas coletadas e numero de projeteis restantes
  tela->geraMapa();
  printStr(22,6,"Projeteis restantes: " + std::to_string(pacMan->getNumProjeteis()));
  printStr(23,6,"Moedas Coletadas: " + std::to_string(pacMan->getMoeda()));
  jogoRodando = true;
      
  //Inicia a musica de fundo
  player->play(sampleFundo,intensities);
  tFundo = get_now_ms();
  T = get_now_ms();
  t1 = T;
  while (!JogadorPerdeu and !JogadorGanhou) {
    // Atualiza timers
    char c = teclado->getchar();	     // Lê o teclado
      t0 = t1;
      t1 = get_now_ms();
      deltaT = t1-t0;

      // Atualiza modelo
      f->update_corpo(deltaT,sentido,direcao);

      //Caso o projetil ainda exista no mapa, atualiza seus calculos de nova posicao
      if(projetil->get_aindaExisto() == true)
        f->update_projetil(deltaT,projetil->get_sentido(),projetil->get_direcao());

      //Atualiza a posicao e velocidade de todos os bots presentes no mapa
      f->update_bot(deltaT, projetil, ultimoComandoFantasma);
      
      // Atualiza tela
      // Enviar Dados daqui

      tela->update(projetil, GameTimeMs + T- t1);

      for (int k = 0; k < 4; k++) {
        statusBots[k] = bot[k]->getStatusVida();
        for (int jj = 0 ; jj < 2 ; jj++)
          posicoesAtuais[k][jj] = (int) bot[k]->get_posicao()[jj];
      }

      positionData->update(pacMan->get_posicao(), posicoesAtuais ,projetil->get_aindaExisto(), projetil->get_posicao(), GameTimeMs + T- t1, pacMan->getMoeda(), pacMan->getNumProjeteis(), 0, statusBots);
      
      std::string buffer(sizeof(DadosImportantes), ' ');
      positionData->serialize(buffer);

      // printStr(24,6,output_buffer);
      for (user_iterator=0; user_iterator<MAX_CONEXOES; user_iterator++){
        if (conexao_usada[user_iterator] == 1){
          if (send(connection_fd[user_iterator], buffer.c_str(), 150, MSG_NOSIGNAL) == -1)  
            remover_conexao(user_iterator);          
        }
      }
          
      
      if (c=='s' or c=='S' or c ==31) {		//Caso em que o jogador deseja descer o player
        sentido = +1;			//sentido positivo (para baixo)
        direcao = 1;			//direcao vertical
      }
      if (c=='w' or c=='w' or c == 30) {	//Caso em que o jogador deseja subir o player
        sentido = -1;			//sentido negativo (para cima)
        direcao = 1;			
      }
      if (c=='a' or c=='A' or c == 17){		//Caso em que o jogador deseja mover o player para a esquerda
        sentido = -1;			//sentido negativo (para para a esquerda)
        direcao = 0;			//direcao horizontal
      }
      if (c=='d' or c=='D' or c == 16){		//Caso em que o jogador deseja mover o player para a direita
        sentido = +1;			//sentido positivo (para direita)
        direcao = 0;
      }
      if (c== 0x20 && pacMan->getNumProjeteis() > 0){  //Verifica se a barra de espaço foi apertada para lançar o projetil e se o jogador ainda possui projeteis
        if(projetil->get_aindaExisto() == false && projetil->get_tenhoProjetil() == true){
          ativaMusicaProjeto = true;
          tPeteleco = get_now_ms();
          player->pause();
          player->play(sampleProjetil,intensities);				//reproduz o som correspondente ao lançamento do projetil
          projetil->status_projetil(true, true);	//seta como true a flag q monitora a existencia do projetil no mapa
          projetil->set_sentido(+sentido);				//salva o sentido inverso do sentido atual do pacman
          projetil->set_direcao(+direcao);				//copia a mesma direcao atual do pacman
          projetil->update_projetil(pacMan->get_velocidade(), pacMan->get_posicao());	//copia a posicao e velocidade atual do player para inicar o movimento do projetil 
          pacMan->DisparaProjeteis();         		//decrementa o numero de projeteis
          printStr(22,6,"Projeteis restantes: " + std::to_string(pacMan->getNumProjeteis()));			//atualiza o print do numero de projeteis restantes
        }
      }
    if (c=='q' or c=='Q') {					//finaliza o jogo imediatamente
      break;
    }

    // Condicao de rebobinar
    // if (sampleFundo->get_position() + 100 > lensampleFundo ) sampleFundo->set_position(0);
    // if (ativaMusicaProjeto){
    //   if ( get_now_ms() - tPeteleco > 500 ){
    //     player->stop();
    //     player->play(sampleFundo,intensities);
    //     ativaMusicaProjeto = false;

    //   }
    // }

    if (get_now_ms() - tFundo > 180000){
      player->play(sampleFundo,intensities);
      tFundo = get_now_ms();
    }
      // std::this_thread::sleep_for (std::chrono::milliseconds(5));
      // i++;
  }

  running = 0;

  if(JogadorGanhou)
    positionData->update(pacMan->get_posicao(), posicoesAtuais ,projetil->get_aindaExisto(), projetil->get_posicao(), GameTimeMs + T- t1, pacMan->getMoeda(), pacMan->getNumProjeteis(), 1, statusBots);
  else
    positionData->update(pacMan->get_posicao(), posicoesAtuais ,projetil->get_aindaExisto(), projetil->get_posicao(), GameTimeMs + T- t1, pacMan->getMoeda(), pacMan->getNumProjeteis(), -1, statusBots);  
      std::string buffer(sizeof(DadosImportantes), ' ');
      positionData->serialize(buffer);

      // printStr(24,6,output_buffer);
      for (user_iterator=0; user_iterator<MAX_CONEXOES; user_iterator++){
        if (conexao_usada[user_iterator] == 1){
          if (send(connection_fd[user_iterator], buffer.c_str(), 150, MSG_NOSIGNAL) == -1)  
            remover_conexao(user_iterator);          
        }
      }

  	//Caso o jogador tenha perdido o jogo, toca a musica de derrota e printa o arquivo telaDerrota na tela
    if (JogadorPerdeu)
    {
      // player->pause();
      player->stop();
      // sampleDerrota->set_position(0);
      player->play(sampleDerrota,intensities);
      std::ifstream file("screens/telaDerrota.txt");
      std::string str;
      int intCount = 9; 
      while (std::getline(file, str))
      {
          printStr(intCount,11,str);
          intCount++;
      }

      std::this_thread::sleep_for (std::chrono::milliseconds(5000));	//Faz a tela de derrota aparecer por 5 segundos
    
    }

    //Caso o jogador tenha vencido o jogo, toca a musica de vitoria e printa o arquivo telaVitoria na tela
    if (JogadorGanhou)
    {
      // player->pause();
      player->stop();
      // sampleDerrota->set_position(0);
      player->play(sampleVitoria,intensities);
      std::ifstream file("screens/telaVitoria.txt");
      std::string str;
      int intCount = 9; 
      while (std::getline(file, str))
      {
          printStr(intCount,15,str);
          intCount++;
      }

      std::this_thread::sleep_for (std::chrono::milliseconds(5000));
    }

  //pausa e finaliza todos os perifericos  
  player->stop();
  tela->stop();
  teclado->stop();
  return 0;
}
