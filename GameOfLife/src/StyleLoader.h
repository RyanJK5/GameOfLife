#ifndef __StyleLoader_h__
#define __StyleLoader_h__

#include <unordered_map>
#include <filesystem>
#include <cctype>
#include <fstream>
#include <expected>
#include <exception>
#include <concepts>

#include "vendor/imgui.h"

namespace gol::StyleLoader
{
	enum class StyleColor
	{
		Transparent, Background, Contrast, Hover, Text
	};

	enum class ErrorType
	{
		FileOpenError, ParseError, InvalidArguments
	};

	enum class SectionType
	{
		None, StyleColors, ImGUIStyle
	};

	struct ErrorState
	{
		ErrorType Type;
		std::string Description;
	};

	class StyleLoaderException : public std::exception
	{
	public:
		StyleLoaderException() : std::exception() {}
		StyleLoaderException(std::string_view str) : std::exception(std::move(str).data()) {}
		StyleLoaderException(const ErrorState& err) : std::exception(err.Description.c_str()) {}
	};

	struct StyleInfo
	{
		std::unordered_map<StyleColor, ImVec4> StyleColors;
		std::unordered_map<ImGuiCol_, StyleColor> AttributeColors;
	};

	static const std::unordered_map<std::string_view, StyleColor> colorDefinitions = {
		{ "transparent", StyleColor::Transparent },
		{ "background", StyleColor::Background },
		{ "contrast", StyleColor::Contrast },
		{ "hover", StyleColor::Hover },
		{ "text", StyleColor::Text }
	};

	static const std::unordered_map<std::string_view, ImGuiCol_> attributeDefinitions = {
		{ "ImGuiCol_WindowBg", ImGuiCol_WindowBg },
		{ "ImGuiCol_Border", ImGuiCol_Border },
		{ "ImGuiCol_Text", ImGuiCol_Text },
		{ "ImGuiCol_Button", ImGuiCol_Button },
		{ "ImGuiCol_ButtonHovered", ImGuiCol_ButtonHovered },
		{ "ImGuiCol_Header", ImGuiCol_Header },
		{ "ImGuiCol_HeaderActive", ImGuiCol_HeaderActive },
		{ "ImGuiCol_TitleBg", ImGuiCol_TitleBg },
		{ "ImGuiCol_TitleBgActive", ImGuiCol_TitleBgActive },
		{ "ImGuiCol_Tab", ImGuiCol_Tab },
		{ "ImGuiCol_TabSelectedOverline", ImGuiCol_TabSelectedOverline },
		{ "ImGuiCol_TabDimmedSelected", ImGuiCol_TabDimmedSelected },
		{ "ImGuiCol_TabHovered", ImGuiCol_TabHovered },
		{ "ImGuiCol_TabUnfocused", ImGuiCol_TabUnfocused },
		{ "ImGuiCol_TabDimmed", ImGuiCol_TabDimmed },
		{ "ImGuiCol_TabSelected", ImGuiCol_TabSelected },
	};

	static const std::unordered_map<std::string_view, SectionType> sectionDefinitions = {
		{ "colors", SectionType::StyleColors },
		{ "imgui-style", SectionType::ImGUIStyle}
	};

	namespace {
		template <typename T>
		std::expected<T, ErrorState> ReadKey(
			int lineNum,
			const std::string& line,
			const std::unordered_map<std::string_view, T> keyMap)
		{
			auto firstLetter = std::find_if(line.begin(), line.end(), [](char c) { return std::isalpha(c); });
			auto seperator = std::find(firstLetter, line.end(), ':');

			if (seperator == line.end())
			{
				return std::unexpected(ErrorState{
					ErrorType::ParseError,
					std::format("Could not find ':' in line {}:\n    {}", lineNum, line)
					});
			}

			auto keyStr = std::string(firstLetter, seperator);
			if (keyMap.count(keyStr) == 0)
			{
				return std::unexpected(ErrorState{
					ErrorType::ParseError,
					std::format("'{}' is not a valid key in line {}:\n    {}", keyStr, lineNum, line)
					});
			}

			return keyMap.at(keyStr);
		}

		void UpdateVec(ImVec4& vec, int index, const std::string& token)
		{
			switch (index)
			{
			case 0: vec.x = std::stof(token); break;
			case 1: vec.y = std::stof(token); break;
			case 2: vec.z = std::stof(token); break;
			case 3: vec.w = std::stof(token); break;
			}
		}

		std::expected<ImVec4, ErrorState> ReadVecValue(int lineNum, const std::string& line, std::string_view values)
		{
			std::string token = "";
			ImVec4 result = {};
			int index = 0;
			for (char c : values)
			{
				if (c == '[' || c == ']' || std::isspace(c))
					continue;
				if (c == ',')
				{
					UpdateVec(result, index, token);
					index++;
					token = "";
					continue;
				}
				token += c;
			}
			UpdateVec(result, index, token);

			if (index < 3)
			{
				return std::unexpected(ErrorState{
					ErrorType::InvalidArguments,
					std::format("Expected 4 parameters, received {} in line {}:\n    {}", index + 1, lineNum, line)
					});
			}

			return result;
		}

		std::expected<std::pair<StyleColor, ImVec4>, ErrorState> ReadColorPair(
			int lineNum,
			const std::string& line,
			const std::string::const_iterator& firstLetter)
		{
			auto seperator = std::find(firstLetter, line.end(), ':');

			auto key = ReadKey<StyleColor>(lineNum, line, colorDefinitions);
			if (!key.has_value())
				return std::unexpected(key.error());

			auto values = std::string_view(seperator + 1, line.end());
			auto value = ReadVecValue(lineNum, line, values);
			if (!value.has_value())
				return std::unexpected(value.error());

			return std::pair<StyleColor, ImVec4> {*key, * value};
		}

		std::expected<std::pair<ImGuiCol_, StyleColor>, ErrorState> ReadImGUIPair(
			int lineNum,
			const std::string& line,
			const std::string::const_iterator& firstLetter)
	{
		auto seperator = std::find(firstLetter, line.end(), ':');

		auto key = ReadKey<ImGuiCol_>(lineNum, line, attributeDefinitions);
		if (!key.has_value())
			return std::unexpected(key.error());

		std::string valueStr = "";
		for (auto it = seperator + 1; it != line.end(); it++)
		{
			if (!std::isspace(*it))
				valueStr += *it;
		}

		if (colorDefinitions.count(valueStr) == 0)
		{
			return std::unexpected(ErrorState{
				ErrorType::ParseError,
				std::format("'{}' is not a valid color in line {}:\n    {}", valueStr, lineNum, line)
				});
		}

		return std::pair<ImGuiCol_, StyleColor> { *key, colorDefinitions.at(valueStr) };
	}
	}

	inline std::expected<StyleInfo, ErrorState> ReadStyle(const std::filesystem::path& styleInfoPath)
	{
		std::ifstream input(styleInfoPath);
		if (!input.is_open())
			return std::unexpected(ErrorState{
				ErrorType::FileOpenError,
				std::format("Could not open file '{}'", styleInfoPath.generic_string())
				});

		std::string line = "";
		int indentWidth = 0;
		int lineNum = 0;

		auto section = SectionType::None;
		auto output = StyleInfo { };

		while (std::getline(input, line))
		{
			lineNum++;

			auto start = std::find_if(line.begin(), line.end(), [](char c) { return std::isalpha(c); });
			if (start == line.end())
				continue;

			if (indentWidth == 0)
				indentWidth = std::distance(line.begin(), start);
			int depth = indentWidth != 0 ? (std::distance(line.begin(), start) / indentWidth) : 0;

			if (depth == 0)
				section = SectionType::None;

			switch (section)
			{
			case SectionType::StyleColors:
			{
				auto result = ReadColorPair(lineNum, line, start);
				if (!result.has_value())
					return std::unexpected(result.error());
				output.StyleColors[result->first] = result->second;
			}	break;
			case SectionType::ImGUIStyle:
			{
				auto result = ReadImGUIPair(lineNum, line, start);
				if (!result.has_value())
					return std::unexpected(result.error());
				output.AttributeColors[result->first] = result->second;
			}	break;
			}

			auto end = std::find_if(line.rbegin(), line.rend(), [](char c) { return !std::isspace(c); });
			if (*end == ':')
			{
				auto sectionHeader = std::string_view(start, end.base());
				if (sectionHeader == "colors:")
					section = SectionType::StyleColors;
				else if (sectionHeader == "imgui-style:")
					section = SectionType::ImGUIStyle;
				else
					return std::unexpected(ErrorState{
						ErrorType::InvalidArguments,
						std::format("'{}' is not a valid section type in line {}:\n    {}", sectionHeader, lineNum, line)
						});
			}
		}

		return output;
	}
}

#endif