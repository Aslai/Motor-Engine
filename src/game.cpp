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
typedef std::basic_string<TCHAR> TString;

#ifdef UNICODE
#define TStrToUTF8(str) Unicode::ToUTF8(str)
#define TStrFromUTF8(str) Unicode::ToUTF16(str)
#else
#define TStrToUTF8(str) str
#define TStrFromUTF8(str) str
#endif

namespace Game{
	class error : public std::exception {
		std::string explain;
	public:
		error(std::string explanation = "") : explain(explanation){
			for (size_t i = 0; i < explain.size();){
				if (explain[i] == '\r'){
					explain.erase(explain.begin() + i, explain.begin() + i + 1);
					continue;
				}
				++i;
			}
		}
		const char * what() const{
			return explain.c_str();
		}
	};
	class not_found_error : public error {
	public:
		not_found_error(std::string name = "") : error("\nCould not find file \"" + name + "\""){

		}
	};

	class MappedFile{
		HANDLE file;
		HANDLE mapping;
		void * view;
		size_t file_size;
		DWORD desiredaccess;
		std::string full_name;
		void MapView(DWORD access){
			desiredaccess = access;
			view = MapViewOfFile(mapping, desiredaccess, 0, 0, 0);
			if (!view){
				throw std::invalid_argument("Failed to map view of file");
			}
		}
	public:
		enum class Access{
			Read,
			ReadWrite
		};

		MappedFile(){
			file = NULL;
			mapping = NULL;
			view = nullptr;
			desiredaccess = NULL;
			file_size = 0;
		}
		MappedFile(std::string fileName, const std::vector<std::string> & include_paths, MappedFile::Access access = Access::Read){
			DWORD waccess = NULL, wprotect = NULL;
			desiredaccess = NULL;
			if (access == Access::Read){
				waccess = GENERIC_READ;
				wprotect = PAGE_READONLY;
				desiredaccess = FILE_MAP_READ;
			}
			else if (access == Access::ReadWrite){
				waccess = GENERIC_WRITE | GENERIC_READ;
				wprotect = PAGE_READWRITE;
				desiredaccess = FILE_MAP_ALL_ACCESS;
			}
			std::wstring wfileName;
			file = INVALID_HANDLE_VALUE;
			for (const auto & path : include_paths){
				wfileName = TStrFromUTF8(path);
				if (wfileName.back() != '\\' && wfileName.back() != '/'){
					wfileName += '\\';
				}
				wfileName += TStrFromUTF8(fileName);
				file = CreateFile(wfileName.c_str(), waccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file != INVALID_HANDLE_VALUE){
					break;
				}
			}
			if (file == INVALID_HANDLE_VALUE){
				wfileName = TStrFromUTF8(fileName);
				file = CreateFile(wfileName.c_str(), waccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			}
			if (file == INVALID_HANDLE_VALUE){
				throw not_found_error(fileName);
			}
			mapping = CreateFileMapping(file, NULL, wprotect, 0, 0, NULL);
			if (!mapping){
				DWORD err = GetLastError();
				throw std::invalid_argument("Failed to create file mapping");
			}
			DWORD size_high;
			DWORD size_low = GetFileSize(file, &size_high);
			file_size = size_high;
			file_size <<= sizeof(DWORD);
			file_size += size_low;

			TCHAR buffer[1000];
			GetFinalPathNameByHandle(file, buffer, 999, VOLUME_NAME_DOS);
			full_name = TStrToUTF8(buffer);

			MapView(desiredaccess);

		}
		MappedFile(std::string fileName, MappedFile::Access access = Access::Read) : MappedFile(fileName, std::vector<std::string>(), access){

		}
		MappedFile& operator=(const MappedFile & other){
			if (!DuplicateHandle(GetCurrentProcess(), other.file, GetCurrentProcess(), &file, 0, FALSE, DUPLICATE_SAME_ACCESS)){
				throw std::invalid_argument("Failed to dup file handle");
			}
			if (!DuplicateHandle(GetCurrentProcess(), other.mapping, GetCurrentProcess(), &mapping, 0, FALSE, DUPLICATE_SAME_ACCESS)){
				throw std::invalid_argument("Failed to dup file mapping");
			}
			MapView(other.desiredaccess);
			file_size = other.file_size;
			full_name = other.full_name;
			return *this;
		}
		MappedFile& operator=(MappedFile && other){
			this->desiredaccess = other.desiredaccess;
			this->file = other.file;
			this->mapping = other.mapping;
			this->view = other.view;
			this->file_size = other.file_size;
			other.file = INVALID_HANDLE_VALUE;
			other.mapping = NULL;
			other.view = nullptr;
			full_name = std::move(other.full_name);
			return *this;
		}
		MappedFile(const MappedFile & other){
			operator=(other);
		}
		MappedFile(MappedFile && other){
			operator=(other);
		}
		~MappedFile(){
			if (view){
				UnmapViewOfFile(view);
			}
			if (mapping){
				CloseHandle(mapping);
			}
			if (file != INVALID_HANDLE_VALUE){
				CloseHandle(file);
			}
		}
		size_t size() const{
			return file_size;
		}
		char& operator[](size_t idx){
			return ((char*)view)[idx];
		}
		char& at(size_t idx) const{
			return ((char*)view)[idx];
		}
		std::string name() const{
			return full_name;
		}
	};
	class Tokenizer{
	public:
		enum class Type{
			None,
			Number,
			String,
			Char,
			Identifier,
			Operator,
			Error
		};
	private:
		const char * data;
		size_t length;
		size_t tok_len;
		Type type;
		std::string state;
		static int isidchar(int value){
			return isalnum(value) || value == '$' || value == '_';
		}
		static int isnumchar(int value){
			return isalnum(value) || value == '.';
		}
	public:
		void set_input(const char * data, size_t len){
			this->data = data;
			length = len;
			tok_len = 0;
			type = Type::None;
		}
		void set_input(const std::string & data){
			set_input(data.c_str(), data.length());
		}
		void set_input(std::string && data){
			state = data;
			set_input(state.c_str(), state.length());
		}
		Tokenizer(){
			set_input(nullptr, 0);
		}
		Tokenizer(const char * data, size_t len){
			set_input(data, len);
		}
		Tokenizer(const std::string & data){
			set_input(data);
		}
		Tokenizer(std::string && data){
			set_input(data);
		}
		Tokenizer(const Tokenizer & other){
			data = other.data;
			length = other.length;
			tok_len = other.tok_len;
			type = other.type;
		}
		Tokenizer(Tokenizer && other){
			data = other.data;
			length = other.length;
			tok_len = other.tok_len;
			type = other.type;
		}
		~Tokenizer(){
			data = nullptr;
		}
		operator std::string(){
			return std::string(data, tok_len);
		}
		operator bool(){
			return tok_len > 0;
		}
		std::string get_string(){
			return (std::string)*this;
		}
		std::string get_remaining(){
			return std::string(data, length);
		}
		bool operator==(const std::string & other){
			return get_string() == other;
		}
		bool operator!=(const std::string & other){
			return get_string() != other;
		}
		Type get_type(){
			return type;
		}
		void print(){
			const char * name;
			switch (type){
			case Type::Char: name = "CHAR"; break;
			case Type::Identifier: name = "ID  "; break;
			case Type::Number: name = "NUM "; break;
			case Type::Operator: name = "OPER"; break;
			case Type::String: name = "STR "; break;
			case Type::Error: name = "ERR "; break;

			default: name = "NONE"; break;
			}
			printf("%s ", name);
			size_t i;
			for (i = 0; i < tok_len; ++i){
				putchar(data[i]);
			}
			putchar('\n');
		}
		Tokenizer & next_token(){
			static const char * token_list[] = {
				"++", "--", "{", "}", "(", ")", "[", "]",
				".", "->", "+", "-", "!", "~", "*", "&",
				"*", "/", "%", "+", "-", ">>", "<<", "<",
				"<=", ">", ">=", "==", "!=", "&", "^", "|",
				"&&", "||", "=", "+=", "-=", "*=", "/=", "%=",
				"<<=", ">>=", "&=", "^=", "|=", ",", ";", "#",
				(const char *)0
			};
			const char ** list = token_list;
			const char * match = NULL;

			data += tok_len;
			if (tok_len > length){
				length = 0;
				tok_len = 0;
				type = Type::Error;
				return *this;
			}
			length -= tok_len;
			while (length > 0 && isspace(data[0])){
				data++;
				length--;
			}
			tok_len = 0;
			if (length == 0){
				type = Type::None;
				return *this;
			}

			const char * string = data;
			type = Type::None;
			size_t match_len = 0;

			while (*list){
				const char * tok_name = *(list++);
				size_t cmp_len = 0;
				while (tok_name && cmp_len < length && tok_name[cmp_len]){
					if (tok_name[cmp_len] != string[cmp_len]){
						break;
					}
					cmp_len++;
				}
				if (tok_name && tok_name[cmp_len] == 0 && cmp_len > match_len){
					match = tok_name;
					match_len = cmp_len;
					type = Type::Operator;
				}
			}
			if (match_len == 0 || (match[0] == '.' && isdigit(string[1]))){
				int escaped = 0;
				if (isdigit(*string) || *string == '.'){
					type = Type::Number;
					while (match_len < length && isnumchar(string[match_len])){
						match_len++;
					}
				}
				else if (*string == '"'){
					type = Type::String;
					while (match_len < length && (match_len == 0 || escaped || string[match_len] != '"')){
						escaped = 0;
						if (string[match_len] == '\\'){
							escaped = 1;
						}
						match_len++;
					}
					match_len++;
				}
				else if (*string == '\''){
					type = Type::Char;
					while (match_len < length && (match_len == 0 || escaped || string[match_len] != '\'')){
						escaped = 0;
						if (string[match_len] == '\\'){
							escaped = 1;
						}
						match_len++;
					}
					match_len++;
				}
				else{
					type = Type::Identifier;
					while (match_len < length && isidchar(string[match_len])){
						match_len++;
					}
				}
			}
			tok_len = match_len;
			return *this;
		}

	};
	class Preprocessor{
	public:
		struct context{
			std::string data;
			std::vector<std::pair<std::string, std::string>> defines;
		};
	private:
		std::string currentData;
		std::string currentSection;
		std::map<std::string, context> sections;
		std::vector<std::vector<std::pair<std::string, std::string>>*> defines_base;
		std::vector<std::pair<std::string, std::string>> defines_base_first;
		std::vector<std::vector<std::pair<std::string, std::string>>*> & defines;
		std::vector<std::string> filenames_base;
		std::vector<std::string> & filenames;
		std::vector<std::string> include_paths_base;
		std::vector<std::string> & include_paths;
		const std::string limitSection;
		MappedFile filedata_base;
		const MappedFile & filedata;

		static void move(std::string & to, const MappedFile & file, size_t start, size_t end){
			size_t originalSize = to.size();
			to.resize(originalSize + end - start);
			memcpy(&to[originalSize], &file.at(start), end - start);
		}
		static void trim_ends(std::string & str){
			str.pop_back();
			str.erase(str.begin(), str.begin() + 1);
		}
		static size_t push_file(std::vector<std::string> & filenames, const std::string & filename){
			for (size_t i = 0; i < filenames.size(); ++i){
				if (filenames[i] == filename){
					return i;
				}
			}
			filenames.push_back(filename);
			return filenames.size() - 1;
		}
		bool Defined(const std::string & section, const std::string & name, std::string & value){
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
		bool Defined(const std::string & section, const std::string & name){
			std::string value;
			return Defined(section, name, value);
		}
		void Define(const std::string & section, const std::string & name, const std::string & value){
			sections[section].defines.push_back(std::make_pair(name, value));
		}
		void Define(const std::string & name, const std::string & value){
			defines.back()->push_back(std::make_pair(name, value));
		}
		bool HandleSection(size_t line_no, Tokenizer & tok){
			if (tok.next_token().get_type() != Tokenizer::Type::String){
				throw error("Section name must be a quoted string; got `" + tok.get_string() + "`.");
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
		bool HandleVersion(size_t line_no, Tokenizer & tok){
			if (!tok.next_token()){
				throw std::invalid_argument("Bad version");
			}
			Define("__INTERNAL VERSION", tok.get_remaining());
			currentData += '\n';
			return true;

		}
		bool HandleInclude(size_t line_no, Tokenizer & tok){
			bool issection = false;
			if (tok.next_token() == "section"){
				issection = true;
				tok.next_token();
			}
			if (tok.get_type() != Tokenizer::Type::String){
				throw error("Include name must be quoted string; got `" + tok.get_string() + "`.");
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
								throw error("String expected after `section`; got `" + tok.get_string() + "`.");
							}
						}
						else{
							throw error("Unknown include specifier `" + tok.get_string() + "`. Did you mean to use `section`?");
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

		bool HandlePragma(size_t line_no, Tokenizer & tok){
			if (!tok.next_token()){
				throw std::invalid_argument("Bad Pragma");
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
		bool HandleToken(size_t line_no, const std::string & line){
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

	public:
		void Include(std::string path){
			include_paths.push_back(path);
		}

		const std::map<std::string, context> & Preprocess(){
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
		const std::string & GetSection(std::string name){
			return sections.find(name)->second.data;
		}
		const std::map<std::string, context> & GetAllSections(){
			return sections;
		}
		Preprocessor() :
			filedata(filedata_base),
			limitSection(limitSection),
			filenames(filenames_base),
			defines(defines_base),
			include_paths(include_paths_base){
			defines_base.push_back(&defines_base_first);
		}
		Preprocessor(const MappedFile & file, const std::string & limitSection = "") :
			filedata(file),
			limitSection(limitSection),
			filenames(filenames_base),
			defines(defines_base),
			include_paths(include_paths_base){
			defines_base.push_back(&defines_base_first);
		}
		Preprocessor(const MappedFile & file, std::vector<std::string> & filenames, const std::string & limitSection = "") :
			filedata(file),
			limitSection(limitSection),
			filenames(filenames),
			defines(defines_base),
			include_paths(include_paths_base){
			defines_base.push_back(&defines_base_first);

		}
		Preprocessor(const MappedFile & file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection = "") :
			filedata(file),
			limitSection(limitSection),
			filenames(filenames),
			defines(defines),
			include_paths(include_paths_base){

		}
		Preprocessor(MappedFile && file, const std::string & limitSection = "") :
			filedata_base(file),
			filedata(filedata_base),
			limitSection(limitSection),
			filenames(filenames_base),
			defines(defines_base),
			include_paths(include_paths_base){
			defines_base.push_back(&defines_base_first);
		}
		Preprocessor(MappedFile && file, std::vector<std::string> & filenames, const std::string & limitSection = "") :
			filedata_base(file),
			filedata(filedata_base),
			limitSection(limitSection),
			filenames(filenames),
			defines(defines_base),
			include_paths(include_paths_base){
			defines_base.push_back(&defines_base_first);

		}
		Preprocessor(MappedFile && file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection = "") :
			filedata_base(file),
			filedata(filedata_base),
			limitSection(limitSection),
			filenames(filenames),
			defines(defines),
			include_paths(include_paths_base){

		}
		std::string TransformGLLog(const std::string & log){
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
	};
	class Shader{
		std::string code;
		std::shared_ptr<Preprocessor> processor;
		GLuint binary;
		GLenum shader_type;
		void Compile(){
			binary = glCreateShader(shader_type);
			if (!binary){
				throw Game::error("Failed to create shader with type " + std::to_string(shader_type));
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

				throw error("Failed to compile GL shader.\n" + info);
			}
		}
		static bool cmp(const std::string & a, const std::string & prefix){
			if (a.substr(0, prefix.size()) == prefix){
				return true;
			}
			return false;
		}
		static bool cmp2(const std::string & a, const std::string & prefix1, const std::string & prefix2){
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

	public:
		Shader(const std::string & name, const std::string & code, std::shared_ptr<Preprocessor> processor) : code(code), processor(processor){
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
		GLuint Get(){
			if (binary == 0){
				Compile();
			}
			return binary;
		}
		operator GLuint(){
			return Get();
		}

	};
	class ShaderFile{
		std::shared_ptr<Preprocessor> processor;
		std::vector<std::string> include_paths;

	public:
		ShaderFile(){
			processor = nullptr;
		}
		~ShaderFile(){

		}
		void LoadFile(std::string filename){
			processor = std::make_shared<Preprocessor>(MappedFile(filename, include_paths), include_paths);
			processor->Preprocess();
		}
		Shader Extract(std::string name){
			return Shader(name, processor->GetSection(name), processor);
		}
		void Include(std::string paths){
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
	};
}



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

