#include <memory>
#include <string>
#include "shaderfile.hpp"
#include "shader.hpp"

namespace Motor{
    ShaderFile::ShaderFile(){
        processor = nullptr;
    }

    ShaderFile::~ShaderFile(){

    }

    void ShaderFile::LoadFile(std::string filename){
        processor = std::make_shared<Preprocessor>(MappedFile(filename, include_paths), include_paths);
        processor->Preprocess();
    }

    Shader ShaderFile::Extract(std::string name){
        return Shader(name, processor->GetSection(name), processor);
    }

    void ShaderFile::Include(std::string paths){
        size_t offset = 0;
        while (offset != std::string::npos){
            size_t newline = paths.find_first_of("\r\n", offset);
            std::string path = paths.substr(offset, newline == std::string::npos ? newline : newline++ - offset);
            if (path.size() > 0){
                printf("path %s\n", path.c_str());
                include_paths.push_back(std::move(path));
            }
            offset = newline;
        }
    }
}

#endif
