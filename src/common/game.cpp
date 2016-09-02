
extern "C" {
	#include SDL2_H
	#undef main
	#include GLEW_H
}

#include "util/exceptions.hpp"
#include "asset/shaderfile.hpp"
#include "math/point.hpp"
#include "asset/audiosrc.hpp"
#include <cstdio>


#include <ctime>
#include "util/stopwatch.hpp"
Motor::AudioMixerLimit limiter(1, 44100.0/2.0*1, 44100.0/2.0 * 3);
void my_audio_proc(void *user, uint8_t *stream, int len) {
	__m128* streamfloats = (__m128*) stream;
	Motor::AudioBuffer & buffer = *(Motor::AudioBuffer*) user;

	len = (len > buffer.Size() * sizeof(float) ? buffer.Size() * sizeof(float) : len);
	//memcpy(stream, buffer.Data(), len);
	for( int i = 0; i < len / sizeof(float); i += 4 ){
        streamfloats[i/4] = limiter.Apply(buffer);
	}
	float peak, avg;
	float avgcnt = 0;
	peak = avg = 0;
	float localpeak = 0;
	size_t highcnt = 0;
	for (int i = 0; i < len / sizeof(float); ++i){
		float sample = abs(buffer.Data()[i]);
		if (sample >= 1){
			highcnt++;
		}
		if (sample > peak){
			peak = sample;
		}
		if (sample > localpeak){
			localpeak = sample;
		}
		if (i % 100 == 0){
			avg += localpeak;
			localpeak = 0;
			avgcnt += 1;
		}
	}
	if (peak >= 1){
		printf("Peak: %f\tAvg: %f\tHighcnt: %llu\n", peak, avg / avgcnt, highcnt);
	}
	buffer.Advance(len / sizeof(float));
}

int main(int argc, char * argv[]){
	try{
		Motor::AudioBuffer buf1, buf2;
		Stopwatch timer;
	    /*char samples1[9] = {1,2,3,4,5,6,7,8,9};
	    float *samples2 = new float[44100*60*60];
        for( int i = 0; i < 44100*60*60; ++i ){
            samples2 [i] = (float)(rand() % 2000) / 1000.0;
        }
        buf1.Push(samples2, 44100*60*60);
        buf2.Push(samples2, 44100*60*60);
        printf("Starting timer...\n");
        //buf2.Push(samples, 10);
        Motor::AudioMixerCompress mix(0.1,100,10000);
        timer.Start();
        mix.Mix2(buf1);
        auto end = timer.Seconds();
        printf("Mix done in %f seconds\n", end);
        timer.Start();
        mix.Mix(buf2);
        end = timer.Seconds();
        printf("Mix2 done in %f seconds\n", end);
        /*for( int i = 0; i < buf1.Size() && i < buf2.Size(); ++i ){
            if( buf1.Data()[i] != buf2.Data()[i]){
                printf("%f\n",buf1.Data()[i] - buf2.Data()[i] );
            }
        }
        buf1.Print();
        printf("\nlmao\n");
        buf2.Print();*/

	    auto p1 = Motor::MakePoint<double>(argc, 10, 3);
	    auto p2 = Motor::MakePoint<double>(10, 4, 2);
	    auto v1 = Motor::MakePoint<double>(10, 4, 1);
	    auto v2 = Motor::MakePoint<double>(-10, 20, 7);
	    printf("%f\n", Motor::PointFindPassTime( p1, p2, v1, v2 ));



		SDL_Window *window;
		SDL_GLContext context;

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
			throw Motor::Exception::Error("Unable to initialize SDL");
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!window){
			//This SDL stuff certainly isn't exception safe ;)
			throw Motor::Exception::Error("Unable to create window");
		}


		context = SDL_GL_CreateContext(window);
		glewInit();

		static uint32_t snd_len;
		static uint8_t *snd_buf;
		static SDL_AudioSpec snd_spec;

		if (SDL_LoadWAV("blue_sky.wav", &snd_spec, &snd_buf, &snd_len) == NULL){
			return 1;
		}
		Motor::AudioBuffer buffer, buffer2;
		if (snd_spec.format == AUDIO_U16){
			buffer.PushNormalized<uint16_t>((uint16_t*)snd_buf, snd_len / 2, 0, UINT16_MAX);
			buffer2.PushNormalized<uint16_t>((uint16_t*)snd_buf, snd_len / 2, 0, UINT16_MAX);
		}
		else if (snd_spec.format == AUDIO_S16){
			buffer.PushNormalized<int16_t>((int16_t*)snd_buf, snd_len / 2, INT16_MIN, INT16_MAX);
			buffer2.PushNormalized<int16_t>((int16_t*)snd_buf, snd_len / 2, INT16_MIN, INT16_MAX);
		}
		/*static uint32_t snd_len2;
		static uint8_t *snd_buf2;
		static SDL_AudioSpec snd_spec2;

		if (SDL_LoadWAV("blue_sky2.wav", &snd_spec2, &snd_buf2, &snd_len2) == NULL){
			return 1;
		}
		Motor::AudioBuffer buffer3;
		if (snd_spec2.format == AUDIO_U16){
			buffer3.PushNormalized<uint16_t>((uint16_t*)snd_buf2, snd_len2 / 2, 0, UINT16_MAX);
		}
		else if (snd_spec2.format == AUDIO_S16){
			buffer3.PushNormalized<int16_t>((int16_t*)snd_buf2, snd_len2 / 2, INT16_MIN, INT16_MAX);
		}
		*/
		Motor::AudioMixerCompress mix2(5, 0.5, 44100.0*2.0*0.2, 44100.0 * 2 * 2);
		Motor::AudioMixerLimit mix3(1, 44100.0*2.0*0.1, 44100.0 * 2 * 1);
		Motor::AudioMixerAmplify mix4(7);
		Motor::AudioMixer mix5;

		timer.Start();
		//mix2.Mix(buffer);
		//mix4.Mix(buffer);
		//auto buffer5 = buffer;
		mix4.Mix(buffer);
		double a = timer.Seconds();
		printf("Mix: %f seconds\n", a);

		timer.Start();
		//mix2.Mix2(buffer2);
		for( size_t i = 0; i < buffer2.Size() / 4; ++i ){
             //mix4.Apply(buffer2[i]);
		}
		//mix4.Mix2(buffer2);
		//mix3.Mix2(buffer2);
		double b = timer.Seconds();
		printf("Apply: %f seconds\n", b);
		printf("%.2f%%slower\n", b/a);

		snd_spec.format = AUDIO_F32;
		snd_spec.callback = my_audio_proc;
		snd_spec.userdata = &buffer;


		SDL_GL_SetSwapInterval(1);

		if (SDL_OpenAudio(&snd_spec, NULL) < 0){
			fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
			exit(-1);
		}
		SDL_PauseAudio(0);

		Motor::ShaderFile s;
		s.Include("..\n../data/");
		s.LoadFile("test.txt");
		Motor::Shader shader = s.Extract("vertex");
		printf("%d\n", shader.Get());


		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);

		while (true){
			SDL_Delay(1000);
		}

		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		//return samples1[0];

	}
	catch (std::exception &e){
		printf("%s\n", e.what());
	}
	while( true ){
        SDL_Delay(1000);
	}
	return 0;
}
