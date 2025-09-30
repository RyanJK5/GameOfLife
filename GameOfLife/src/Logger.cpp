#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <ranges>

#include "Logger.h"

gol::log::Logger::Logger(const std::unordered_set<LogCode>& activeCodes, bool active)
	: Active(active)
{
	for (int i = LogCode::GLError; i <= LogCode::Info; i++)
	{
		m_codes[static_cast<LogCode>(i)] = false;
	}
	for (LogCode code : activeCodes)
	{
		m_codes[code] = true;
	}
}

gol::log::Logger::Logger(LogCode level, bool active)
	: Active(active)
{	
	for (int i = LogCode::GLError; i <= LogCode::Info; i++)
	{
		m_codes[static_cast<LogCode>(i)] = i <= level;
	}
}

static std::string ErrorRepresentation(gol::log::LogCode code)
{
	using namespace gol::log;
	switch (code)
	{
	case LogCode::Error:
		return "[ERROR]";
	case LogCode::GLError:
		return "[GL ERROR]";
	case LogCode::Warning:
		return "[WARNING]";
	case LogCode::Info:
		return "[INFO]";
	default:
		throw std::invalid_argument("Invalid code provided");
	}
}

static std::string SimplifyFileName(const std::string& fileName)
{
	size_t nameStart = fileName.find_last_of('\\');
	if (nameStart > 0 && nameStart < fileName.size() - 1)
	{
		return fileName.substr(nameStart + 1);
	}
	return fileName;
}

static bool SkipAngleBrackets(const std::string& expression, size_t& pos, const std::unordered_set<std::string> skip)
{
	if (expression[pos] != '<')
		return false;

	bool inSkip = false;
	for (auto& token : skip)
	{
		size_t tokenBegin = pos - token.size();
		if (expression.find(token, tokenBegin) == tokenBegin)
		{
			inSkip = true;
			break;
		}
	}
	if (!inSkip)
		return false;

	int openAngles = 1;
	while (pos < expression.length())
	{
		pos++;
		if (expression[pos] == '<')
		{
			openAngles++;
		}
		else if (expression[pos] == '>')
		{
			openAngles--;
		}

		if (openAngles == 0)
		{
			break;
		}
	}
	
	return true;
}

static bool SkipTokens(const std::string& expression, size_t& pos, const std::unordered_set<std::string> skip)
{
	for (auto& token : skip)
	{
		if (expression.find(token, pos) == pos)
		{
			pos += token.length();
			if (expression[pos] != ' ')
				pos--;
			return true;
		}
	}
	return false;
}

static std::string SimplifyFunctionName(const std::string& funcName)
{
	std::unordered_set<std::string> skip = { "class", "__cdecl", "std::"};
	std::unordered_set<std::string> allowTemplates = { "basic_string" };
	
	std::string result = "";
	for (size_t i = 0; i < funcName.length(); i++)
	{
		if (SkipTokens(funcName, i, skip))
			continue;
		if (SkipAngleBrackets(funcName, i, allowTemplates))
			continue;

		result += funcName[i];
	}
	return result;
}

void gol::log::Logger::Log(LogCode code, const std::string& message, const std::source_location& location)
{
	if (!m_codes[code] || !Active)
		return;
	
	if (Trace)
	{
		std::cerr << ErrorRepresentation(code) 
		  		  << " at " << SimplifyFileName(location.file_name())
				  << ":" << location.line() 
				  << " in " << SimplifyFunctionName(location.function_name())
		 		  << std::endl;
	}
	if (message.size() > 0)
	{
		std::cerr << message << std::endl;
		std::cerr << std::endl;
	}
}

void gol::log::Logger::LogError(const std::string& message, const std::source_location& location)
{
	Log(Error, message, location);
}

void gol::log::Logger::LogWarning(const std::string& message, const std::source_location& location)
{
	Log(Warning, message, location);
}

void gol::log::Logger::LogInfo(const std::string& message, const std::source_location& location)
{
	Log(Info, message, location);
}

void gol::log::Logger::LogGLErrors(const std::string& function, const std::source_location& location)
{
	if (!Active || !ActiveLevel(LogCode::GLError))
	{
		return;
	}

	while (GLenum error = glGetError())
	{
		Log(LogCode::GLError, "Error Code " + std::to_string(error), location);
	}
}