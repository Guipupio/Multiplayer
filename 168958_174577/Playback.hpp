#pragma once

#define N_CHANNELS 2
#define MAX_SAMPLES 256

#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <portaudio.h>
#include <sndfile.h>

namespace Audio{

	class Sample{
		private:
			int n_channels;
			unsigned long int n_frames;

		public:
			float *data;

			Sample(const char *filename);
			~Sample();

			unsigned long int get_n_frames();
			int get_n_channels();

	};


	class Player {
		private:
			bool playing;

			PaStreamParameters  outputParameters;
			PaStream*           stream;
			PaError             err;
			PaTime              streamOpened;

            struct play_element {
                Sample *samplePointer;
                unsigned long int pos;
                float intensities[N_CHANNELS];
                unsigned int flags;
                struct sample_queue *next;
            };




		public:
			struct play_element sample_vector[MAX_SAMPLES];//FIXME should not be public

            // Default constructor and destructors:
			Player();
			~Player();

            // Callback function for portaudio, should not be called by user:
			static int PA_Callback (const void *inputBuffer, void *outputBuffer,
							 unsigned long framesPerBuffer,
							 const PaStreamCallbackTimeInfo* timeInfo,
							 PaStreamCallbackFlags statusFlags,
							 void *userData );


			void pause();
            void resume();
			void stop();
            // Play a sample, receives sample pointer and intensities vector of length N_CHANNELS for each channel intensity
			int play(Sample *audiosample, float *intensities);
            // Blocks until all samples finish playing:
            void block();
            // Block until a specified sample is finished. Receives index, which identifies which sample to wait for, obtained from play()
            void block(int index);
	};
}
