
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
class Stopwatch{
    struct timespec start;
    public:
    Stopwatch();
    void Start();
    double Seconds();
    unsigned long long Nanoseconds();
};
#include <time.h>

Stopwatch::Stopwatch(){

}
void Stopwatch::Start(){
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
}
double Stopwatch::Seconds(){
    struct timespec tr, tn;
    clock_getres(CLOCK_THREAD_CPUTIME_ID, &tr);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tn);
    double s = (tn.tv_sec - start.tv_sec);
    s *= 1000000000;
    s += tn.tv_nsec;
    s -= start.tv_nsec;
    s /= 1000000000;
    return s;
}
unsigned long long Stopwatch::Nanoseconds(){
    struct timespec tr, tn;
    clock_getres(CLOCK_THREAD_CPUTIME_ID, &tr);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tn);
    unsigned long long s = (tn.tv_sec - start.tv_sec);
    s *= 1000000000;
    s += tn.tv_nsec;
    s -= start.tv_nsec;
    return s;
}


int main(int argc, char * argv[]){
	try{
	    Motor::AudioBuffer buf1, buf2;
	    char samples1[9] = {1,2,3,4,5,6,7,8,9};
	    float *samples2 = new float[44100*60*60];
        for( int i = 0; i < 44100*1/*60*60*/; ++i ){
            samples2 [i] = (float)(rand() % 2000) / 1000.0;
        }
        buf1.Push(samples2, 44100);
        buf2.Push(samples2, 44100);
        printf("Starting timer...\n");
        //buf2.Push(samples, 10);
        Stopwatch timer;
        Motor::AudioMixerCompress<3,1,1,2,4> mix;
        timer.Start();
        mix.Mix(buf1);
        auto end = timer.Seconds();
        printf("Mix done in %f seconds\n", end);
        timer.Start();
        mix.Mix2(buf2);
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

		if (SDL_Init(SDL_INIT_VIDEO) < 0){
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


		SDL_GL_SetSwapInterval(1);

		Motor::ShaderFile s;
		s.Include("..\n../data/");
		s.LoadFile("test.txt");
		Motor::Shader shader = s.Extract("vertex");
		printf("%d\n", shader.Get());


		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);

		SDL_Delay(2000);

		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return samples1[0];

	}
	catch (std::exception &e){
		printf("%s\n", e.what());
	}
	while( true ){
        SDL_Delay(1000);
	}
	return 0;
}
