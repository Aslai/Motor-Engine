
extern "C" {
	#include SDL2_H
	#undef main
	#include GLEW_H
}

#include "util/exceptions.hpp"
#include "asset/shaderfile.hpp"
#include "math/point.hpp"
#include <cstdio>


int main(int argc, char * argv[]){
	try{
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

	}
	catch (std::exception &e){
		printf("%s\n", e.what());
	}
	while( true ){
        SDL_Delay(1000);
	}
	return 0;
}
