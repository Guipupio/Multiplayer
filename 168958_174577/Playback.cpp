#include "Playback.hpp"

using namespace Audio;

Sample::Sample(const char *filename){
	SNDFILE * infile;
	SF_INFO sfinfo;

	float buffer[2520];

	if (! (infile = sf_open (filename, SFM_READ, &sfinfo))){
		printf ("Not able to open input file %s.\n", filename) ;
		/* Print the error message from libsndfile. */
		puts (sf_strerror (NULL)) ;

		n_channels = 0;
		n_frames = 0;
		data = NULL;
		return;
	}

	n_frames = sfinfo.frames;
	// Aceitamos no maximo 2 canais:
	//n_channels = sfinfo.channels > 2 ? 2 : sfinfo.channels;
	n_channels = N_CHANNELS;

	// Alocar buffer do tamanho necessario:
	data = (float *)malloc(n_frames * n_channels * sizeof(float));

	int read_bytes;
	int offset = 0;
	int channel = 0;
	do{
		read_bytes = sf_read_float(infile, buffer, 2520);
		for (int i = 0; i < read_bytes; i++){
			//printf("%f\n", buffer[i]);


			if (channel < n_channels){
				data[offset] = buffer[i];
				offset++;
			}
			// Se o arquivo nao tem canais o suficiente para oque queremos, completar com o ultimo canal:
			if (offset % n_channels >= sfinfo.channels){
				for (int j = 0; j < n_channels - sfinfo.channels; j++){
					data[offset] = buffer[i];
					offset++;
				}
			}
			channel = (channel + 1) % sfinfo.channels;
		}

	}while(read_bytes == 2520);

	sf_close(infile);

}

Sample::~Sample(){
	if (data != NULL)
		free(data);
}

unsigned long int Sample::get_n_frames(){
	return n_frames;
}

int Sample::get_n_channels(){
	return n_channels;
}


Player::Player() {
	this->playing = false;

	PaError err;

	err = Pa_Initialize();
	if( err != paNoError ) {
		std::cerr << "Error on Pa_Initialize()" << std::endl;
		return;
	}

	outputParameters.device = Pa_GetDefaultOutputDevice();// [> Default output device. <]
	if (outputParameters.device == paNoDevice) {
		std::cerr << "Error: No default output device on Pa_GetDefaultOutputDevice()" << std::endl;
		return;
	}

	outputParameters.channelCount = N_CHANNELS;                   //  [> Mono output. <]
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	err = Pa_OpenStream( &stream,
						 NULL,    //  [> No input. <]
						 &outputParameters,
						 46000,
						 1024,      // [> Frames per buffer. <]
						 paClipOff,// [> We won't output out of range samples so don't bother clipping them. <]
						 &Player::PA_Callback,
						 this );

	if( err != paNoError ) {
		std::cerr << "Error on Pa_OpenStream()" << std::endl;
		return;
	}

	for (int i = 0; i < MAX_SAMPLES; i++){
		sample_vector[i].samplePointer = NULL;
	}


}

Player::~Player() {
	pause();

	err = Pa_CloseStream( stream );
	if( err != paNoError ) {
		std::cerr << "Error on Pa_StopStream()" << std::endl;
		return;
	}

	Pa_Terminate();
}

int Player::play(Sample *audiosample, float *intensities){
    
	int index = -1;
	for (int i = 0; i < MAX_SAMPLES && index == -1; i++){
		if (!sample_vector[i].samplePointer){
			sample_vector[i].samplePointer = audiosample;
			sample_vector[i].pos = 0;
			memcpy(sample_vector[i].intensities, intensities, N_CHANNELS * sizeof(float));
			sample_vector[i].flags = 0;
			index = i;
		}
	}
    //struct sample_queue *newSample = (struct sample_queue*)malloc(sizeof(struct sample_queue));
	
    if (!playing){
        err = Pa_StartStream( stream );
        if( err != paNoError ) {
            std::cerr << "Error on Pa_StartStream()" << std::endl;
			sample_vector[index].samplePointer = NULL;
            return -1;
        }
        this->playing = true;
    }

	return index;
}

void Player::pause() {
    err = Pa_StopStream( stream );
    if( err != paNoError ){
        std::cerr << "Error on Pa_StopStream()" << std::endl;
        return;
    }

    this->playing = false;
}

void Player::stop() {
	PaError err;
	err = Pa_StopStream( stream );
	if( err != paNoError ) {
		std::cerr << "Error on Pa_StopStream()" << std::endl;
		return;
	}
    this->playing = false;

    for (int i = 0; i < MAX_SAMPLES; i++){
        sample_vector[i].samplePointer = NULL;
    }


}


int Player::PA_Callback (const void *inputBuffer, void *outputBuffer,
				 unsigned long framesPerBuffer,
				 const PaStreamCallbackTimeInfo* timeInfo,
				 PaStreamCallbackFlags statusFlags,
				 void *userData ){

	Player *player = (Player*) userData;
	float *buffer = (float *) outputBuffer;

	memset(outputBuffer, 0, sizeof(float) * framesPerBuffer * N_CHANNELS);

	int somethingPlaying = 0;
	for (int i = 0; i < MAX_SAMPLES; i++){
		if (player->sample_vector[i].samplePointer != NULL){
			somethingPlaying = 1;
			for (int j = player->sample_vector[i].pos; j < player->sample_vector[i].samplePointer->get_n_frames() && j < framesPerBuffer + player->sample_vector[i].pos; j++){
				for (int k = 0; k < N_CHANNELS; k++){
					buffer[j - player->sample_vector[i].pos + k] += player->sample_vector[i].samplePointer->data[j * N_CHANNELS + k] * player->sample_vector[i].intensities[k];
					//buffer[j - player->sample_vector[i].pos + k] = player->sample_vector[i].samplePointer->data[j * N_CHANNELS + k] * 1;
				}
			}
			player->sample_vector[i].pos += framesPerBuffer;
			if (player->sample_vector[i].pos >= player->sample_vector[i].samplePointer->get_n_frames()){
				player->sample_vector[i].samplePointer = NULL;
			}
		}
	}
	if (!somethingPlaying){
		//player->playing = false;
		//return paComplete;
	}
	return 0;

}
