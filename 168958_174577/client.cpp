#include "Playback.hpp"
#include "Teclado.hpp"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <thread>
#include <chrono>


std::vector<std::string> mapa;
char playerName = '@';

int socket_fd;
bool jogoRolando = true;
char comando;

Tela *tela = new Tela(40, 40, 40, 40);

using namespace std::chrono;
uint64_t get_now_ms() {
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}


void *threadfun (void *parametros)
{
  char c;
  // timeout(1);
  while (jogoRolando) {
    c = getch();
    if (c== 's' or c == 'w' or c == 'a' or c=='d' or c == 'q') comando = c;
    else (comando) = 0;
    std::this_thread::sleep_for (std::chrono::milliseconds(10));

  }
  
}

void *receber_respostas(void *parametros) {
  /* Recebendo resposta */
  char reply[512];
  int msg_len;
  int msg_num;
  msg_num = 0;
  while(jogoRolando) {
    msg_len = recv(socket_fd, reply, 150, MSG_DONTWAIT);
    if (msg_len > 0) {
      tela->unserialize(reply);
    }
  }
}


void geraMapa(){
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

int main() {

  tela->init();

  float intensities[2] = {1.0,1.0};
  uint64_t tFundo = 0;
  uint64_t t0;

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

  t0 = get_now_ms();
  // Espera
  while (1) {
    std::this_thread::sleep_for (std::chrono::milliseconds(1));
    
    if (get_now_ms() - t0 > 6500) break;
  }


  //Apos iniciar a tela, imprime o nome do jogo, presente no arquivo telaInit.txt
  std::ifstream file("screens/telaInit.txt");
    std::string str;
    int intCount = 3, msglen; 
    while (std::getline(file, str))
    {
        printStr(intCount,5,str);
        intCount++;
    }

  struct sockaddr_in target;
  pthread_t receiver, newthread;
  char mensagem[2];
  char input_buffer[3];
  
  jogoRolando = true;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  //printf("Socket criado\n");

  target.sin_family = AF_INET;
  target.sin_port = htons(3001);
  inet_aton("127.0.0.1", &(target.sin_addr));
  //printf("Tentando conectar\n");
  if (connect(socket_fd, (struct sockaddr*)&target, sizeof(target)) != 0) {
    std::cout << "Problemas na conexao"<< '\n';
    return 0;
  }

  //Aguarda conecoes dos demais jogadores

  while (true) {
    
    msglen = recv(socket_fd, input_buffer, 3, MSG_DONTWAIT);
    if (msglen > 0) {
      if (input_buffer[0] == 'O' and input_buffer[1] == 'K'){
        playerName = input_buffer[2];
        break;
      }
    }
  }

    // Contagem regressiva para inicio do jogo

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

  geraMapa();
  
  pthread_create(&receiver, NULL, receber_respostas, NULL);
  // Inicializa ncurses
  raw();                 /* Line buffering disabled */
  keypad(stdscr, TRUE);  /* We get F1, F2 etc..   */
  noecho();              /* Don't echo() while we do getch */
  curs_set(0);           /* Do not display cursor */

  pthread_create(&newthread, NULL, threadfun, NULL);
  // std::thread newthread(threadfun);
  // kb_thread.swap(newthread);


  player->play(sampleFundo,intensities);       //reproduz o som correspondente ao lançamento do projetil

  tFundo = get_now_ms();
  while(jogoRolando) {
  /* Agora, meu socket funciona como um descritor de arquivo usual */
    tela->update();
    char c = comando;
    if (c== 's' or c == 'w' or c == 'a' or c=='d'){
      sprintf(mensagem,"%c",c);
      send(socket_fd, mensagem, 2, MSG_NOSIGNAL);
    }
    
    if(tela->posicoes.statusJogo == 0 and c != 'q'){
      jogoRolando = true;
    }
    else
      jogoRolando = false;

    // ativaMusicaProjeto = true;
    // tPeteleco = get_now_ms();
    // player->pause();
    // player->play(sampleProjetil,intensities);       //reproduz o som correspondente ao lançamento do projetil

    if (get_now_ms() -tFundo > 180000){
      player->play(sampleFundo,intensities);      
      tFundo = get_now_ms();
    }

    // std::this_thread::sleep_for (std::chrono::milliseconds(5));
  }

  pthread_join(newthread,NULL);
  pthread_join(receiver, NULL);
  //Caso o jogador tenha perdido o jogo, toca a musica de derrota e printa o arquivo telaDerrota na tela
    if (tela->posicoes.statusJogo == 1 or tela->posicoes.statusJogo == 0)
    {
      // player->pause();
      player->stop();
      player->play(sampleDerrota,intensities);
      std::ifstream file("screens/telaDerrota.txt");
      std::string str;
      int intCount = 9; 
      while (std::getline(file, str))
      {
          printStr(intCount,11,str);
          intCount++;
      }

      std::this_thread::sleep_for (std::chrono::milliseconds(5000));  //Faz a tela de derrota aparecer por 5 segundos
    
    }    //Caso o jogador tenha vencido o jogo, toca a musica de vitoria e printa o arquivo telaVitoria na tela
    else{
      // player->pause();    
      player->stop();
      player->play(sampleVitoria,intensities);
      std::ifstream file("screens/telaVitoria.txt");
      std::string str;
      int intCount = 9; 
      while (std::getline(file, str))
      {
          printStr(intCount,11,str);
          intCount++;
      }

      std::this_thread::sleep_for (std::chrono::milliseconds(5000));
    }

  // kb_thread.join();
  tela->stop();
  close(socket_fd);
  
  return 0;
}