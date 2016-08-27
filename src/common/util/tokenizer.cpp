#include "tokenizer.hpp"
#include <string>
#include <cctype>

namespace Motor{
    int Tokenizer::isidchar(int value){
        return isalnum(value) || value == '$' || value == '_';
    }

    int Tokenizer::isnumchar(int value){
        return isalnum(value) || value == '.';
    }



    Tokenizer::Tokenizer(){
        set_input(nullptr, 0);
    }

    Tokenizer::Tokenizer(const char * data, size_t len){
        set_input(data, len);
    }

    Tokenizer::Tokenizer(const std::string & data){
        set_input(data);
    }

    Tokenizer::Tokenizer(std::string && data){
        set_input(data);
    }

    Tokenizer::Tokenizer(const Tokenizer & other){
        data = other.data;
        length = other.length;
        tok_len = other.tok_len;
        type = other.type;
    }

    Tokenizer::Tokenizer(Tokenizer && other){
        data = other.data;
        length = other.length;
        tok_len = other.tok_len;
        type = other.type;
    }

    Tokenizer::~Tokenizer(){
        data = nullptr;
    }

    void Tokenizer::set_input(const char * data, size_t len){
        this->data = data;
        length = len;
        tok_len = 0;
        type = Type::None;
    }

    void Tokenizer::set_input(const std::string & data){
        set_input(data.c_str(), data.length());
    }

    void Tokenizer::set_input(std::string && data){
        state = data;
        set_input(state.c_str(), state.length());
    }

    Tokenizer::operator std::string(){
        return std::string(data, tok_len);
    }

    Tokenizer::operator bool(){
        return tok_len > 0;
    }

    std::string Tokenizer::get_string(){
        return (std::string)*this;
    }

    std::string Tokenizer::get_remaining(){
        return std::string(data, length);
    }

    bool Tokenizer::operator==(const std::string & other){
        return get_string() == other;
    }

    bool Tokenizer::operator!=(const std::string & other){
        return get_string() != other;
    }

    Tokenizer::Type Tokenizer::get_type(){
        return type;
    }

    void Tokenizer::print(){
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

    Tokenizer & Tokenizer::next_token(){
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
}
