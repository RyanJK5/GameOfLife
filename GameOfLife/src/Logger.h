#pragma once
#include <unordered_map>
#include <string>
#include <source_location>
#include <unordered_set>

#ifdef _DEBUG
	#ifdef LOGGER
		#define GL_DEBUG(statement) \
			while (LOGGER.Active && LOGGER.ActiveLevel(gol::log::GLError) && glGetError());\
			statement;\
			LOGGER.LogGLErrors(#statement)

		#define LOG(code, str) LOGGER.Log(code, str)
	#else
		#define GL_DEBUG(statement) statement
		#define LOG(code, str)
	#endif
#else
	#define GL_DEBUG(statement) statement
	#define LOG(code, str)
#endif

namespace gol::log
{
	enum LogCode
	{
		GLError,
		Error,
		Warning,
		Info
	};

	class Logger
	{
	public:
		explicit Logger(const std::unordered_set<LogCode>& activeCodes, bool active = true);

		explicit Logger(LogCode level, bool active = true);

		void Log(LogCode code, const std::string& message = "", const std::source_location& location = std::source_location::current());

		void LogError(const std::string& message = "", const std::source_location& location = std::source_location::current());
		void LogWarning(const std::string& message = "", const std::source_location& location = std::source_location::current());
		void LogInfo(const std::string& message = "", const std::source_location& location = std::source_location::current());


		void LogGLErrors(const std::string& call, const std::source_location& location = std::source_location::current());

		inline bool ActiveLevel(LogCode code) const { return m_codes.at(code); }

		inline void EnableLevel(LogCode code) { m_codes[code] = true; }
		inline void DisableLevel(LogCode code) { m_codes[code] = false; }
	public:
		bool Trace = true;
		bool Active;
	private:
		std::unordered_map<LogCode, bool> m_codes;
	};

}