#include "shader.hpp"
#include "preprocessor.hpp"
extern "C"{
    #include GLEW_H
}
#include <memory>
#include "util/exceptions.hpp"
#include <climits>

namespace Motor{
    void Shader::Compile(){
        binary = glCreateShader(shader_type);
        if (!binary){
            throw Exception::Error("Failed to create shader with type " + std::to_string(shader_type));
        }
        GLchar * glcode = (GLchar*)code.c_str();
        GLint code_len = 0;
        if (code.size() < (size_t)INT_MAX){
            code_len = (GLint)code.size();
        }
        glShaderSource(binary, 1, (const GLchar **)&glcode, &code_len);
        glCompileShader(binary);

        GLint compiled = 0;
        glGetShaderiv(binary, GL_COMPILE_STATUS, &compiled);
        if (compiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(binary, GL_INFO_LOG_LENGTH, &maxLength);
            std::string info;
            info.resize(maxLength);
            glGetShaderInfoLog(binary, maxLength, &maxLength, &info[0]);

            glDeleteShader(binary);
            binary = 0;
            info = processor->TransformGLLog(info);

            throw Exception::Error("Failed to compile GL shader.\n" + info);
        }
    }

    bool Shader::cmp(const std::string & a, const std::string & prefix){
        if (a.substr(0, prefix.size()) == prefix){
            return true;
        }
        return false;
    }

    bool Shader::cmp2(const std::string & a, const std::string & prefix1, const std::string & prefix2){
        if (cmp(a, prefix1)){
            size_t location = a.find_first_of(prefix2);
            if (location != std::string::npos && location > 0){
                if (isspace(a[location - 1]) || a[location - 1] == '_' || prefix1.size() + 3 >= location){
                    return true;
                }
            }
        }
        return false;
    }



    Shader::Shader(const std::string & name, const std::string & code, std::shared_ptr<Preprocessor> processor) : code(code), processor(processor){
        std::string type = name.substr(0, name.find_first_of('_'));
        if (cmp(name, "comp")){
            shader_type = GL_COMPUTE_SHADER;
        }
        else if (cmp(name, "vert") || cmp(name, "vrt")){
            shader_type = GL_VERTEX_SHADER;
        }
        else if (cmp2(name, "tes", "ct") || cmp2(name, "tes", "co")){
            shader_type = GL_TESS_CONTROL_SHADER;
        }
        else if (cmp2(name, "tes", "ev")){
            shader_type = GL_TESS_EVALUATION_SHADER;
        }
        else if (cmp(type, "geo")){
            shader_type = GL_GEOMETRY_SHADER;
        }
        else if (cmp(type, "pix") || cmp(type, "frag")){
            shader_type = GL_FRAGMENT_SHADER;
        }
        binary = 0;
    }

    const GLuint & Shader::Get(){
        if (binary == 0){
            Compile();
        }
        return binary;
    }

    Shader::operator GLuint(){
        return Get();
    }

    void Shader::Preload(){
        if (binary == 0){
            Compile();
        }
    }
}
