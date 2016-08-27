#ifndef MOTOR_H_ASSET_SHADERFILE_HPP
#define MOTOR_H_ASSET_SHADERFILE_HPP

#include <memory>
#include <string>
#include "shader.hpp"

namespace Motor{
	class ShaderFile{
	public:
		ShaderFile();
		~ShaderFile();
		void LoadFile(std::string filename);
		Shader Extract(std::string name);
		void Include(std::string paths);

    private:
		std::shared_ptr<Preprocessor> processor;
		std::vector<std::string> include_paths;
	};
}

#endif
