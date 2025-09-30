#pragma once
#include <exception>

namespace gol
{
	class GLException : public std::exception
	{
	public:
		GLException() : std::exception() { }

		GLException(const std::string& str) : std::exception(str.c_str()) { }
};
}
