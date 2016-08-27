#include <cstdint>
#include <vector>
#include <string>
#include <exception>
#include "unicode.h"

namespace Motor{
    namespace Unicode{
        uint32_t Mask(int bits){
            return bits-- <= 0 ?
                0 :
                (1 << bits) | Mask(bits);
        }

        bool Compare(char c, int maskbits){
            return (uint8_t)(c & ~Mask(maskbits)) == (uint8_t)~Mask(maskbits + 1);
        }

        uint32_t ReadCodepoint(std::string::const_iterator & iter, std::string::const_iterator end){
            size_t len = 1;
            uint32_t codepoint = 0;
            if (Compare(*iter, 7)){
                len = 1;
                codepoint = *iter;
            }
            else if (Compare(*iter, 5)){
                len = 2;
                codepoint = (*iter & Mask(5)) << 6;
            }
            else if (Compare(*iter, 4)){
                len = 3;
                codepoint = (*iter & Mask(4)) << 12;
            }
            else if (Compare(*iter, 3)){
                len = 4;
                codepoint = (*iter & Mask(3)) << 18;
            }
            else if (Compare(*iter, 2)){
                len = 5;
                codepoint = (*iter & Mask(2)) << 24;
            }
            else if (Compare(*iter, 1)){
                len = 6;
                codepoint = (*iter & Mask(1)) << 30;
            }
            while (--len > 0 && ++iter != end){
                codepoint |= (*iter & Mask(6)) << (6 * (len-1));
            }
            if (len > 0){
                throw Exception::Error("Tried indexing codepoint over end of UTF8 string.");
            }
            return codepoint;
        }

        void AppendCodepoint(std::string & str, uint32_t codepoint){
            if ((codepoint & Mask(7)) == codepoint){
                str.push_back((codepoint >> 0) & Mask(7));
            }
            else if ((codepoint & Mask(11)) == codepoint){
                str.push_back(((codepoint >> 6) & Mask(5)) | ~Mask(6));
                str.push_back(((codepoint >> 0) & Mask(6)) | ~Mask(7));
            }
            else if ((codepoint & Mask(16)) == codepoint){
                str.push_back(((codepoint >> 12) & Mask(4)) | ~Mask(5));
                str.push_back(((codepoint >> 6) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 0) & Mask(6)) | ~Mask(7));
            }
            else if ((codepoint & Mask(21)) == codepoint){
                str.push_back(((codepoint >> 18) & Mask(3)) | ~Mask(4));
                str.push_back(((codepoint >> 12) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 6) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 0) & Mask(6)) | ~Mask(7));
            }
            else if ((codepoint & Mask(26)) == codepoint){
                str.push_back(((codepoint >> 24) & Mask(2)) | ~Mask(3));
                str.push_back(((codepoint >> 18) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 12) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 6) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 0) & Mask(6)) | ~Mask(7));
            }
            else if ((codepoint & Mask(31)) == codepoint){
                str.push_back(((codepoint >> 30) & Mask(1)) | ~Mask(2));
                str.push_back(((codepoint >> 24) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 18) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 12) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 6) & Mask(6)) | ~Mask(7));
                str.push_back(((codepoint >> 0) & Mask(6)) | ~Mask(7));
            }
        }

        uint32_t ReadCodepoint(std::wstring::const_iterator & iter, std::wstring::const_iterator end){
            if (*iter < 0xD800 || (*iter >= 0xE000 && *iter < 0x10000)){
                return *iter;
            }
            else{
                size_t codepoint = (*(iter++) & Mask(10)) << 10;
                codepoint |= *(iter) & Mask(10);
                return codepoint;
            }
        }

        void AppendCodepoint(std::wstring & str, uint32_t codepoint){
            if (codepoint < 0xD800 || (codepoint >= 0xE000 && codepoint < 0x10000) ){
                str.push_back(codepoint);
            }
            else if (codepoint >= 0x10000 && codepoint < 0x10FFFF){
                codepoint -= 0x10000;
                str.push_back(((codepoint >> 10) & Mask(10)) | 0xD800);
                str.push_back(((codepoint >> 0) & Mask(10)) | 0xDC00);
            }
            else{
                throw Exception::Error("Failed to encode codepoint " + std::to_string(codepoint));
            }
        }

        std::string ToUTF8(const wchar_t * const & str){
            std::string ret = "";
            for (size_t i = 0; str[i]; ++i){
                AppendCodepoint(ret, str[i]);
            }
            return ret;
        }

        std::string ToUTF8(const wchar_t * const & str, size_t length){
            std::string ret = "";
            ret.reserve(length);
            for (size_t i = 0; i < length; ++i){
                AppendCodepoint(ret, str[i]);
            }
            return ret;
        }

        std::wstring ToUTF16(std::string const & str){
            std::wstring ret = L"";
            ret.reserve(str.length());
            for (auto iter = str.begin(); iter != str.end(); ++iter){
                uint32_t codepoint = ReadCodepoint(iter, str.end());
                AppendCodepoint(ret, codepoint);
            }
            return ret;
        }

        std::string ToUTF8(std::wstring const & str){
            std::string ret = "";
            ret.reserve(str.length());
            for (auto iter = str.begin(); iter != str.end(); ++iter){
                uint32_t codepoint = ReadCodepoint(iter, str.end());
                AppendCodepoint(ret, codepoint);
            }
            return ret;
        }


    }
}
