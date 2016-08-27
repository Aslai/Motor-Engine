#include "fs/mappedfile.hpp"

namespace Motor{
    void Preprocessor::move(std::string & to, const MappedFile & file, size_t start, size_t end){
        size_t originalSize = to.size();
        to.resize(originalSize + end - start);
        memcpy(&to[originalSize], &file.at(start), end - start);
    }

    void Preprocessor::trim_ends(std::string & str){
        str.pop_back();
        str.erase(str.begin(), str.begin() + 1);
    }

    size_t Preprocessor::push_file(std::vector<std::string> & filenames, const std::string & filename){
        for (size_t i = 0; i < filenames.size(); ++i){
            if (filenames[i] == filename){
                return i;
            }
        }
        filenames.push_back(filename);
        return filenames.size() - 1;
    }

    bool Preprocessor::Defined(const std::string & section, const std::string & name, std::string & value){
        auto iter = sections.find(section);
        if (iter != sections.end()){
            for (size_t i = iter->second.defines.size(); i-- > 0;){
                if (iter->second.defines[i].first == name){
                    value = iter->second.defines[i].second;
                    return true;
                }
            }
        }
        for (size_t i = defines.size(); i--> 0;){
            for (size_t j = defines[i]->size(); j-- > 0;){
                if (defines[i]->at(j).first == name){
                    value = defines[i]->at(j).second;
                    return true;
                }
            }
        }
        return false;
    }

    bool Preprocessor::Defined(const std::string & section, const std::string & name){
        std::string value;
        return Defined(section, name, value);
    }

    void Preprocessor::Define(const std::string & section, const std::string & name, const std::string & value){
        sections[section].defines.push_back(std::make_pair(name, value));
    }

    void Preprocessor::Define(const std::string & name, const std::string & value){
        defines.back()->push_back(std::make_pair(name, value));
    }

    bool Preprocessor::HandleSection(size_t line_no, Tokenizer & tok){
        if (tok.next_token().get_type() != Tokenizer::Type::String){
            throw Exception::Error("Section name must be a quoted string; got `" + tok.get_string() + "`.");
        }
        size_t file_number = push_file(filenames, filedata.name());
        std::string version;
        std::string prefix;
        if (Defined(currentSection, "__INTERNAL VERSION", version) && defines.size() == 1){
            prefix += "#version " + version + "\n";
        }
        if (currentSection == ""){
            prefix += "#line 1 " + std::to_string(file_number) + "\n";
        }
        sections[currentSection].data = prefix + currentData;
        currentData = "";

        currentData += "#line " + std::to_string(line_no) + " " + std::to_string(file_number) + "\n";
        std::string section = tok;
        section.pop_back();
        section.erase(section.begin(), section.begin() + 1);
        currentSection = section;
        if (currentSection == "__END"){
            return true;
        }
        return true;
    }

    bool Preprocessor::HandleVersion(size_t line_no, Tokenizer & tok){
        if (!tok.next_token()){
            throw Exception::Error("Bad version");
        }
        Define("__INTERNAL VERSION", tok.get_remaining());
        currentData += '\n';
        return true;

    }

    bool Preprocessor::HandleInclude(size_t line_no, Tokenizer & tok){
        bool issection = false;
        if (tok.next_token() == "section"){
            issection = true;
            tok.next_token();
        }
        if (tok.get_type() != Tokenizer::Type::String){
            throw Exception::Error("Include name must be quoted string; got `" + tok.get_string() + "`.");
        }
        MappedFile mfile;
        std::string section = "";
        if (issection){
            section = tok;
            trim_ends(section);
            mfile = filedata;
        }
        else{
            std::string file = tok;
            trim_ends(file);
            size_t pos = file.find_first_of("?");
            if (pos != std::string::npos){
                section = file.substr(pos + 1);
                file.erase(file.begin() + pos, file.end());
            }
            else{
                if (tok.next_token()){
                    if (tok == "section"){
                        if (tok.next_token().get_type() == Tokenizer::Type::String){
                            section = tok;
                            trim_ends(section);
                        }
                        else{
                            throw Exception::Error("String expected after `section`; got `" + tok.get_string() + "`.");
                        }
                    }
                    else{
                        throw Exception::Error("Unknown include specifier `" + tok.get_string() + "`. Did you mean to use `section`?");
                    }
                }
            }
            mfile = MappedFile(file, include_paths);
        }
        size_t file_number;// = push_file(filenames, mfile.name());
        std::string test_name = "___ONCE_INCLUDE " + section + "?" + mfile.name();
        if (Defined(section, test_name)){
            currentData.push_back('\n');
            return true;
        }
        //currentData += "#line 1 " + std::to_string(file_number) + "\n";
        defines.push_back(&sections[currentSection].defines);
        Preprocessor p = Preprocessor(mfile, defines, filenames, section);
        p.include_paths = include_paths;
        auto newSections = p.Preprocess();
        auto& back = *defines.back();
        defines.pop_back();
        for (auto & def : back){
            Define(def.first, def.second);
        }
        file_number = push_file(filenames, filedata.name());
        currentData += newSections.at(section).data;
        currentData.push_back('\n');
        currentData += "#line " + std::to_string(line_no) + " " + std::to_string(file_number) + "\n";
        return true;
    }

    bool Preprocessor::HandlePragma(size_t line_no, Tokenizer & tok){
        if (!tok.next_token()){
            throw Exception::Error("Bad Pragma");
        }
        if (tok == "once"){
            std::string name = "___ONCE_INCLUDE " + currentSection + "?" + filedata.name();
            if (currentSection == limitSection){
                Define(name, "");
                currentData += '\n';
            }
            return true;
        }

        return false;
    }

    bool Preprocessor::HandleToken(size_t line_no, const std::string & line){
        Tokenizer tok;
        tok.set_input(line);
        if (tok.next_token() != "#"){
            return false;
        }
        if (!tok.next_token()){
            return false;
        }
        if (tok == "section"){
            return HandleSection(line_no, tok);
        }
        if (limitSection == "" || currentSection == limitSection){
            if (tok == "version"){
                return HandleVersion(line_no, tok);
            }
            if (tok == "pragma"){
                return HandlePragma(line_no, tok);
            }
            if (tok == "include"){
                return HandleInclude(line_no, tok);
            }
        }
        return false;
    }



    void Preprocessor::Include(std::string path){
        include_paths.push_back(path);
    }

    const Preprocessor::std::map<std::string, context> & Preprocess(){
        bool foundChar = false;
        bool preprocessing = false;
        size_t line_no = 1;
        size_t copied = 0;
        size_t newline = 0;
        for (size_t i = 0; i < filedata.size(); ++i){
            if (!isspace(filedata.at(i))){
                if (!foundChar && filedata.at(i) == '#'){
                    move(currentData, filedata, copied, newline);
                    copied = newline;
                    preprocessing = true;
                }
                foundChar = true;
            }
            if (filedata.at(i) == '\n'){
                ++line_no;
                if (preprocessing){
                    std::string line;
                    move(line, filedata, newline, i);
                    if (HandleToken(line_no, line)){
                        copied = i + 1;
                    }
                }
                foundChar = false;
                newline = i + 1;
                preprocessing = false;
            }
        }
        move(currentData, filedata, copied, filedata.size());
        HandleToken(line_no, "#section \"__END\"");
        return sections;
    }

    const std::string & Preprocessor::GetSection(std::string name){
        return sections.find(name)->second.data;
    }

    const std::map<std::string, context> & Preprocessor::GetAllSections(){
        return sections;
    }

    Preprocessor::Preprocessor() :
        filedata(filedata_base),
        limitSection(limitSection),
        filenames(filenames_base),
        defines(defines_base),
        include_paths(include_paths_base){
        defines_base.push_back(&defines_base_first);
    }
    Preprocessor::Preprocessor(const MappedFile & file, const std::string & limitSection) :
        filedata(file),
        limitSection(limitSection),
        filenames(filenames_base),
        defines(defines_base),
        include_paths(include_paths_base){
        defines_base.push_back(&defines_base_first);
    }

    Preprocessor::Preprocessor(const MappedFile & file, std::vector<std::string> & filenames, const std::string & limitSection) :
        filedata(file),
        limitSection(limitSection),
        filenames(filenames),
        defines(defines_base),
        include_paths(include_paths_base){
        defines_base.push_back(&defines_base_first);

    }

    Preprocessor::Preprocessor(const MappedFile & file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection) :
        filedata(file),
        limitSection(limitSection),
        filenames(filenames),
        defines(defines),
        include_paths(include_paths_base){

    }

    Preprocessor::Preprocessor(MappedFile && file, const std::string & limitSection) :
        filedata_base(file),
        filedata(filedata_base),
        limitSection(limitSection),
        filenames(filenames_base),
        defines(defines_base),
        include_paths(include_paths_base){
        defines_base.push_back(&defines_base_first);
    }

    Preprocessor::Preprocessor(MappedFile && file, std::vector<std::string> & filenames, const std::string & limitSection) :
        filedata_base(file),
        filedata(filedata_base),
        limitSection(limitSection),
        filenames(filenames),
        defines(defines_base),
        include_paths(include_paths_base){
        defines_base.push_back(&defines_base_first);

    }

    Preprocessor::Preprocessor(MappedFile && file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection) :
        filedata_base(file),
        filedata(filedata_base),
        limitSection(limitSection),
        filenames(filenames),
        defines(defines),
        include_paths(include_paths_base){

    }

    std::string Preprocessor::TransformGLLog(const std::string & log){
        std::string ret;
        size_t offset = log.length() > 0 ? 0 : std::string::npos;
        while (offset != std::string::npos){
            try{
                size_t end = log.find_first_of('\n', offset + 1);
                size_t paren = offset;
                while (true){
                    paren = log.find_first_of('(', paren + 1);
                    if (paren >= end){
                        break;
                    }
                    size_t bound_begin = log.find_last_not_of("()0123456789", paren);
                    bound_begin = bound_begin == std::string::npos ? 0 : bound_begin + 1;
                    size_t bound_end = log.find_first_not_of("()0123456789", paren);
                    bound_end = bound_end == std::string::npos ? end : bound_end;
                    std::string file_num_str = log.substr(bound_begin, paren - bound_begin);
                    ++paren;
                    std::string line_num_str = log.substr(paren, bound_end - paren);
                    if (line_num_str.size() == 0 || line_num_str.back() != ')'){
                        continue;
                    }
                    line_num_str.pop_back();

                    size_t file_num = std::strtoul(file_num_str.c_str(), nullptr, 10);
                    size_t line_num = std::strtoul(line_num_str.c_str(), nullptr, 10);
                    ret += log.substr(offset, bound_begin - offset);

                    if (file_num < filenames.size()){
                        ret += filenames[file_num];
                    }
                    else{
                        ret += "Unkown";
                    }
                    ret += ":" + std::to_string(line_num);
                    offset = bound_end;
                }
                if (end != std::string::npos){
                    ret += log.substr(offset, end - offset);
                    ++end;
                }
                else{
                    ret += log.substr(offset);
                }
                ret += "\n";
                offset = end;
            }
            catch (std::exception e){
                return ret;
            }
        }
        return ret;
    }
}
