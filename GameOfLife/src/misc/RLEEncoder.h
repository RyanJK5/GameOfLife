#ifndef __RLEEncoder_h__
#define __RLEEncoder_h__

#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <stdexcept>
#include <vector>

#include "GameGrid.h"
#include "Graphics2D.h"

namespace gol::RLEEncoder
{
	template <std::unsigned_integral StorageType>
	static constexpr StorageType FormatNumber(StorageType value)
	{
		constexpr StorageType getData   = 0b00111111;
		constexpr StorageType indicator = 0b01000000;
		StorageType result = 0b0;
		
		for (int8_t i = 0; i < sizeof(StorageType); i++)
		{
			result |= ((getData << (i * 6)) & value) << (i * 2);
			result |= indicator << (i * 8);
		}

		return result;
	}

	template <std::unsigned_integral StorageType>
	static constexpr std::vector<StorageType> FormatDimension(uint32_t dim)
	{
		uint32_t formatted = FormatNumber<uint32_t>(dim);
		std::vector<StorageType> result;
		for (int8_t i = sizeof(uint32_t) - sizeof(StorageType); i >= 0; i -= sizeof(StorageType))
			result.push_back(formatted >> (i * 8U));
		std::ranges::reverse(result);
		return result;
	}

	template <std::unsigned_integral StorageType>
	static constexpr StorageType ReadNumber(const char* value)
	{
		constexpr uint8_t getData = 0b00111111;
		StorageType result = 0b0;

		for (int8_t i = 0; i < sizeof(StorageType); i++)
		{
			result |= (getData & static_cast<const uint8_t>(value[i])) << (i * 6);
		}
		return result;
	}

	template <std::unsigned_integral StorageType>
	inline std::vector<StorageType> EncodeRegion(const GameGrid& grid, const Rect& region)
	{
		constexpr StorageType largestValue = std::numeric_limits<StorageType>::max() >> (2 * sizeof(StorageType));

		auto encoded = std::vector<StorageType> {};
		encoded.append_range(FormatDimension<StorageType>(region.Width));
		encoded.append_range(FormatDimension<StorageType>(region.Height));
		encoded.emplace_back(FormatNumber<StorageType>('0'));

		gol::Vec2 runStart = { region.X, region.Y - 1 };
		bool running = false;
		bool first = true;
		for (const auto& pos : grid.Data())
		{
			if (!region.InBounds(pos))
				continue;

			if (!running)
			{
				StorageType count = (region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) - 1;
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				if (count > 0)
					encoded.emplace_back(FormatNumber<StorageType>(count));
				else if (first)
					encoded[2 * sizeof(uint32_t) / sizeof(StorageType)] = FormatNumber<StorageType>('1');
				running = true;
				runStart = pos;
			}

			first = false;
			gol::Vec2 nextPos = { pos.X, pos.Y + 1 };
			if (nextPos.Y >= region.Y + region.Height)
			{
				nextPos.X++;
				nextPos.Y = region.Y;
			}

			if (!region.InBounds(nextPos) || grid.Data().find(nextPos) == grid.Data().end())
			{
				StorageType count = (region.Height * (pos.X - runStart.X) + pos.Y - runStart.Y) + 1;
				if (count > largestValue)
					throw std::invalid_argument("Specified region contains data that is too large");
				encoded.emplace_back(FormatNumber<StorageType>(count));
				running = false;
				runStart = pos;
			}
		}
		if (!running)
		{
			gol::Vec2 pos = region.LowerRight();
			StorageType count = (region.Height * (pos.X - 1 - runStart.X) + pos.Y - runStart.Y) - 1;
			if (count > largestValue)
				throw std::invalid_argument("Specified region contains data that is too large");
			encoded.push_back(FormatNumber<StorageType>(count));
		}

		if (encoded.size() > 2)
		{
			encoded.push_back('\0');
			return encoded;
		}
		return { '\0' };
	}

	template <std::unsigned_integral StorageType>
	inline std::expected<gol::GameGrid, StorageType> DecodeRegion(const char* data, StorageType warnThreshold)
	{
		if (data[0] == '\0')
			return {};

		uint32_t width = ReadNumber<uint32_t>(data);
		uint32_t height = ReadNumber<uint32_t>(data + sizeof(uint32_t));
		if (width * height > std::numeric_limits<StorageType>::max() >> 2)
		{
			return std::unexpected { std::numeric_limits<StorageType>::max() };
		}

		bool running = ReadNumber<StorageType>(data + 2 * sizeof(uint32_t)) == '1';
		StorageType xPtr = 0;
		StorageType yPtr = 0;

		GameGrid result(width, height);

		StorageType warnCount = 0;
		for (size_t i = 2 * sizeof(uint32_t) + sizeof(StorageType); data[i] != '\0'; i += sizeof(StorageType))
		{
			StorageType count = ReadNumber<StorageType>(data + i);
			if (running)
				warnCount += count;
			if (warnCount >= warnThreshold)
			{
				running = !running;
				continue;
			}

			if (running)
			{
				for (StorageType j = 0; j < count; j++)
				{
					result.Set(
						xPtr + (yPtr + j) / height,
						(yPtr + j) % height,
						true
					);
				}
			}

			running = !running;
			xPtr += (yPtr + count) / height;
			yPtr = (yPtr + count) % height;
		}

		if (warnCount >= warnThreshold)
			return std::unexpected { warnCount };
		return std::expected<GameGrid, StorageType> { std::move(result) };
	}

	constexpr std::variant<uint8_t, uint16_t, uint32_t, uint64_t> SelectStorageType(uint64_t count)
	{
		if (count <= std::numeric_limits<uint8_t>::max() >> 2)
			return uint8_t {};
		else if (count <= std::numeric_limits<uint16_t>::max() >> 2)
			return uint16_t {};
		else if (count <= std::numeric_limits<uint32_t>::max() >> 2)
			return uint32_t {};
		return uint64_t {};
	}
}

#endif