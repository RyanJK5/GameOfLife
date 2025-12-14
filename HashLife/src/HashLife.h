#ifndef __HashLife_h__
#define __HashLife_h__

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <unordered_map>

#include "Node.h"

namespace gol
{
	namespace hashlife
	{
		template <typename T>
		concept Vec2 = std::signed_integral<decltype(T::X)>
					&& std::signed_integral<decltype(T::Y)>
					&& std::totally_ordered<T>;

		template <typename Range, typename T>
		concept RangeOf = std::ranges::range<Range> && std::same_as<T, std::ranges::range_value_t<Range>>;
	}

	class HashLife
	{
	public:
		HashLife(const std::vector<bool>& data, int32_t width, int32_t height);

		const Node& NextGeneration(const Node& node, int32_t level);
	private:
		const Node* BuildQuadtree(const std::vector<bool>& data,
			int32_t left, int32_t right, int32_t top, int32_t bottom,
			int32_t width, int32_t height);

		const Node& RetrieveOrInsert(const Node& node);

		const Node& CenteredSubnode(const Node* node);
		const Node& CenteredHorizontal(const Node* east, const Node* west);
		const Node& CenteredVertical(const Node* north, const Node* south);
		const Node& CenteredSubSubnode(const Node* node);

		const Node& CalculateBaseCase(const Node& node);

		int32_t CountNeighbors(uint32_t row, uint32_t col, const std::vector<bool>& contents);
	private:
		const Node* Root;
		int32_t RootLevel = 1;

		std::unordered_map<Node, const Node*, NodeHasher> m_Cache;
	};
}

#endif