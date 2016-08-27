#ifndef MOTOR_H_UTIL_TOKENIZER_HPP
#define MOTOR_H_UTIL_TOKENIZER_HPP

#include <string>

namespace Motor{
    class Tokenizer{
		static int isidchar(int value);
		static int isnumchar(int value);

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

		void set_input(const char * data, size_t len);
		void set_input(const std::string & data);
		void set_input(std::string && data);
		Tokenizer();
		Tokenizer(const char * data, size_t len);
		Tokenizer(const std::string & data);
		Tokenizer(std::string && data);
		Tokenizer(const Tokenizer & other);
		Tokenizer(Tokenizer && other);
		~Tokenizer();
		operator std::string();
		operator bool();
		std::string get_string();
		std::string get_remaining();
		bool operator==(const std::string & other);
		bool operator!=(const std::string & other);
		Type get_type();
		void print();
		Tokenizer & next_token();

	private:
		const char * data;
		size_t length;
		size_t tok_len;
		Type type;
		std::string state;

	};
}

#endif
