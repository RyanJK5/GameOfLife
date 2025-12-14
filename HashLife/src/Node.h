#ifndef __Node_h__
#define __Node_h__

#include <cstdint>
#include <variant>
#include <memory>

namespace gol
{
	struct Node
	{
		uint64_t Hash = 0;

		const Node* NorthWest;
		const Node* NorthEast;
		const Node* SouthEast;
		const Node* SouthWest;

		Node(const Node* nw, const Node* ne, const Node* sw, const Node* se)
			: NorthWest(nw), NorthEast(ne), SouthWest(sw), SouthEast(se)
		{
			Hash = reinterpret_cast<uint64_t>(nw) ^ reinterpret_cast<uint64_t>(ne) ^
				   reinterpret_cast<uint64_t>(sw) ^ reinterpret_cast<uint64_t>(se);
		}

		Node(bool nw, bool ne, bool sw, bool se)
			: NorthWest(nullptr), NorthEast(nullptr), SouthWest(nullptr), SouthEast(nullptr)
		{
			Hash = nw | (ne << 1) | (sw << 2) | (se << 3);
		}
	};

	struct NodeHasher
	{
		size_t operator()(const Node& node) const { return node.Hash; }
	};
};

#endif