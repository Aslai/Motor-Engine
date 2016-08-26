#include <string>

namespace Unicode{
	std::string ToUTF8(const wchar_t * const & str);
	std::string ToUTF8(const wchar_t * const & str, size_t length);
	std::string ToUTF8(std::wstring const & str);
	std::wstring ToUTF16(std::string const & str);
	std::string ToUTF8(std::wstring const & str);
}