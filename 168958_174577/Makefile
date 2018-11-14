FLAGS=-lncurses -lpthread -lportaudio libsndfile.a -lasound -std=c++11 -Wwrite-strings

all:
	 g++ -O0 -g3 -o server jogo_main.cpp jogoSP.cpp Playback.cpp  $(FLAGS) && g++ -O0 -g3 -o client client.cpp Teclado.cpp Playback.cpp $(FLAGS)

server: jogo_main.cpp jogoSP.cpp jogoSP.hpp Playback.cpp Playback.hpp
	g++ -O0 -g3 -o server jogo_main.cpp jogoSP.cpp Playback.cpp  $(FLAGS)

client: client.cpp Teclado.cpp Teclado.hpp
	g++ -O0 -g3 -o client client.cpp Teclado.cpp $(FLAGS)

clean:
	rm -f server && rm -f client
