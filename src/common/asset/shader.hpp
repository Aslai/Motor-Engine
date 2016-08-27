#ifndef MOTOR_H_ASSET_SHADER_HPP
#define MOTOR_H_ASSET_SHADER_HPP

#include "preprocessor.hpp"
extern "C"{
    #include GLEW_H
}
#include <memory>
#include "util/exceptions.hpp"


namespace Motor{
	class Shader{
		void Compile();
		static bool cmp(const std::string & a, const std::string & prefix);
		static bool cmp2(const std::string & a, const std::string & prefix1, const std::string & prefix2);

	public:
		Shader(const std::string & name, const std::string & code, std::shared_ptr<Preprocessor> processor);
		const GLuint & Get();
		operator GLuint();

    private:
		std::string code;
		std::shared_ptr<Preprocessor> processor;
		GLuint binary;
		GLenum shader_type;
	};
}

#endif
