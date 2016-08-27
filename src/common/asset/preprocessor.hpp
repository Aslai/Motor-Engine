#ifndef MOTOR_H_ASSET_PREPROCESSOR
#define MOTOR_H_ASSET_PREPROCESSOR

#include "fs/mappedfile.hpp"
#include "util/tokenizer.hpp"
#include <string>
#include <vector>
#include <map>

namespace Motor{
	class Preprocessor{
	    static void move(std::string & to, const MappedFile & file, size_t start, size_t end);
		static void trim_ends(std::string & str);
		static size_t push_file(std::vector<std::string> & filenames, const std::string & filename);
		bool Defined(const std::string & section, const std::string & name, std::string & value);
		bool Defined(const std::string & section, const std::string & name);
		void Define(const std::string & section, const std::string & name, const std::string & value);
		void Define(const std::string & name, const std::string & value);
		bool HandleSection(size_t line_no, Tokenizer & tok);
		bool HandleVersion(size_t line_no, Tokenizer & tok);
		bool HandleInclude(size_t line_no, Tokenizer & tok);
		bool HandlePragma(size_t line_no, Tokenizer & tok);
		bool HandleToken(size_t line_no, const std::string & line);

	public:
		struct context{
			std::string data;
			std::vector<std::pair<std::string, std::string>> defines;
		};

		void Include(std::string path);

		const std::map<std::string, context> & Preprocess();
		const std::string & GetSection(std::string name);
		const std::map<std::string, context> & GetAllSections();
		Preprocessor();
		Preprocessor(const MappedFile & file, const std::string & limitSection = "");
		Preprocessor(const MappedFile & file, std::vector<std::string> & filenames, const std::string & limitSection = "");
		Preprocessor(const MappedFile & file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection = "");
		Preprocessor(MappedFile && file, const std::string & limitSection = "");
		Preprocessor(MappedFile && file, std::vector<std::string> & filenames, const std::string & limitSection = "");
		Preprocessor(MappedFile && file, std::vector<std::vector<std::pair<std::string, std::string>>*> & defines, std::vector<std::string> & filenames, const std::string & limitSection = "");
		std::string TransformGLLog(const std::string & log);

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
	};
}

#endif
