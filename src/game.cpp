//
// AGPL... For now.
// 
// (c) Andrew Story
//     All rights reserved
//

extern "C" {
	#include <windows.h>
	#include <SDL.h>
	#undef main
	#include <glew.h>
}

#include <glm/mat4x4.hpp>
#include <map>
#include <string>
#include <vector>
#include <exception>
#include <cstdio>
#include <cstdlib>

#include "unicode.h"
#include <memory>




int main(){
	try{
		//Smash::MappedFile file = Smash::MappedFile("Smash.cpp", Smash::MappedFile::Access::ReadWrite);

		std::vector<std::string> filenames;
		Game::Tokenizer tok = Game::Tokenizer("for( int i =  0; i< 1000; ++i){ return \"ayyyyy''' LMAO\";}");
		tok.set_input("  # include \"stdio.h\"  ");
		while (tok.next_token()){
			tok.print();
		}
		Game::MappedFile file = Game::MappedFile("test.txt", Game::MappedFile::Access::ReadWrite);
		auto p = Game::Preprocessor(file, filenames);
		p.Include("..");
		auto sections = p.Preprocess();
		for (auto & section : sections){
			printf("SECTION \"%s\"\n%s\n", section.first.c_str(), section.second.data.c_str());
		}



		SDL_Window *window;
		SDL_GLContext context; 

		if (SDL_Init(SDL_INIT_VIDEO) < 0){
			throw Game::error("Unable to initialize SDL");
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!window){
			//This SDL stuff certainly isn't exception safe ;)
			throw Game::error("Unable to create window");
		}

		
		context = SDL_GL_CreateContext(window);
		glewInit();


		SDL_GL_SetSwapInterval(1);

		Game::ShaderFile s;
		s.Include("..\n");
		s.LoadFile("test.txt");
		Game::Shader shader = s.Extract("vertex");
		printf("%d\n", shader.Get());



		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);

		SDL_Delay(20000000);

		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();

	}
	catch (std::exception &e){
		printf("%s\n", e.what());
	}
	Sleep(1000000);
	return 0;
}

