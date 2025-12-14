#include "HashLife.h"
#include "Node.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

gol::HashLife::HashLife(const std::vector<bool>& data, int32_t width, int32_t height)
{
	auto size = 0b1 << std::max(4, static_cast<int32_t>(std::ceil(std::log2(std::max(width, height)))));
	Root = BuildQuadtree(data, 
		0, 0, size - 1, size - 1,
		width, height);
}

const gol::Node* gol::HashLife::BuildQuadtree(const std::vector<bool>& data,
	int32_t left, int32_t right, int32_t top, int32_t bottom,
	int32_t width, int32_t height)
{
	if (right - left + 1 == 2)
	{
		return Node{
			data[left * width + width / 2],
			data[left * width + 1],
			data[left * (width + 1)],
			data[left * (width + 1) + 1]
		}
	}

	return &RetrieveOrInsert(Node
	{
		BuildQuadtree(data, left, (left + right) / 2, top, (top + bottom) / 2, width, height),
		BuildQuadtree(data, (left + right) / 2 + 1, right, top, (top + bottom) / 2, width, height),
		BuildQuadtree(data, left, (left + right) / 2, (top + bottom) / 2 + 1, bottom, width, height),
		BuildQuadtree(data, (left + right) / 2 + 1, right, (top + bottom) / 2 + 1, bottom, width, height)
	});
}

const gol::Node& gol::HashLife::RetrieveOrInsert(const Node& node)
{
	auto itr = m_Cache.find(node);
	if (itr != m_Cache.end())
		return itr->first;

	m_Cache.insert({ node, nullptr });
	return m_Cache.find(node)->first;
}

const gol::Node& gol::HashLife::CenteredSubnode(const Node* node)
{
	return RetrieveOrInsert(Node
	{
		node->NorthWest->SouthEast,
		node->NorthEast->SouthWest,
		node->SouthWest->NorthEast,
		node->SouthEast->NorthWest
	});
}

const gol::Node& gol::HashLife::CenteredHorizontal(const Node* east, const Node* west)
{
	return RetrieveOrInsert(Node
	{
		west->NorthEast->SouthEast,
		east->NorthWest->SouthWest,
		west->SouthEast->NorthEast,
		east->SouthWest->NorthWest
	});
}

const gol::Node& gol::HashLife::CenteredVertical(const Node* north, const Node* south)
{
	return RetrieveOrInsert(Node
	{
		north->SouthWest->SouthEast,
		north->SouthEast->SouthWest,
		south->NorthWest->NorthEast,
		south->NorthEast->NorthWest
	});
}

const gol::Node& gol::HashLife::CenteredSubSubnode(const Node* node)
{
	return RetrieveOrInsert(Node
	{
		node->NorthWest->SouthEast->SouthEast,
		node->NorthEast->SouthWest->SouthWest,
		node->SouthWest->NorthEast->NorthEast,
		node->SouthEast->NorthWest->NorthWest
	});
}

const gol::Node& gol::HashLife::CalculateBaseCase(const Node& node)
{
	// TODO optimize with bitwise operations

	constexpr auto size = 4uz;
	std::vector<bool> contents { size * size };
	auto nodes = std::array<const Node*, size> { node.NorthWest, node.NorthEast, node.SouthWest, node.SouthEast };
	for (auto i = 0uz; i < nodes.size(); i++)
	{
		auto bit = 0b1uz;
		for (auto j = 0uz; j < nodes.size(); j++)
		{
			bool alive = (bit << 1uz) & nodes[i]->Hash;
			auto row = i / 2 + j / 2;
			auto col = i % 2 * 2 + j % 2;
			contents[row * 4 + col] = alive;
		}
	}

	std::vector<bool> result {};
	result.reserve(size * size);
	for (auto row = 0uz; row < size; row++)
	{
		for (auto col = 0uz; col < size; col++)
		{
			auto neighbors = CountNeighbors(row, col, contents);
			result.push_back(neighbors == 3 || (neighbors == 2 && contents[row * size + col]));
		}
	}

	const auto& ogNode = RetrieveOrInsert(Node {
		&RetrieveOrInsert(Node { contents[0], contents[1], contents[size], contents[size] }),
		&RetrieveOrInsert(Node { contents[2], contents[3], contents[size + 2], contents[size + 3] }),
		&RetrieveOrInsert(Node { contents[2 * size], contents[2 * size + 1], contents[3 * size], contents[3 * size + 1] }),
		&RetrieveOrInsert(Node { contents[2 * size + 2], contents[2 * size + 3], contents[3 * size + 2], contents[3 * size + 3] })
	});
	const auto& newNode = RetrieveOrInsert(Node {
		&RetrieveOrInsert(Node { result[0], result[1], result[size], result[size] }),
		&RetrieveOrInsert(Node { result[2], result[3], result[size + 2], result[size + 3] }),
		&RetrieveOrInsert(Node { result[2 * size], result[2 * size + 1], result[3 * size], result[3 * size + 1] }),
		&RetrieveOrInsert(Node { result[2 * size + 2], result[2 * size + 3], result[3 * size + 2], result[3 * size + 3] })
	});
	
	m_Cache[ogNode] = &newNode;
	return newNode;
}

int32_t gol::HashLife::CountNeighbors(uint32_t row, uint32_t col, const std::vector<bool>& contents)
{
	auto neighbors = 0;
	for (auto i = -1z; i <= 1z; i++)
	{
		for (auto j = -1z; j <= 1z; j++)
		{
			if (i == 0 && j == 0)
				continue;
			auto nRow = row + i;
			auto nCol = col + i;
			if (nRow < 0 || nRow >= 4 || nCol < 0 || nCol >= 4)
				continue;
			if (contents[nRow * 4 + nCol])
				neighbors++;
		}
	}
	return neighbors;
}


const gol::Node& gol::HashLife::NextGeneration(const Node& node, int32_t level)
{
	auto itr = m_Cache.find(node);
	if (itr != m_Cache.end())
		return *itr->second;

	if (level == 2)
	{
		return CalculateBaseCase(node);
	}

	const auto& n00 = CenteredSubnode(node.NorthWest);
	const auto& n01 = CenteredHorizontal(node.NorthWest, node.NorthEast);
	const auto& n02 = CenteredSubnode(node.NorthEast);
	const auto& n10 = CenteredVertical(node.NorthWest, node.SouthWest);
	const auto& n11 = CenteredSubSubnode(&node);
	const auto& n12 = CenteredVertical(node.NorthEast, node.SouthEast);
	const auto& n20 = CenteredSubnode(node.SouthWest);
	const auto& n21 = CenteredHorizontal(node.SouthWest, node.SouthEast);
	const auto& n22 = CenteredSubnode(node.SouthEast);

	const auto& result = RetrieveOrInsert(Node
	{
		&NextGeneration(Node { &n00, &n01, &n10, &n11 }, level - 2),
		&NextGeneration(Node { &n01, &n02, &n11, &n12 }, level - 2),
		&NextGeneration(Node { &n10, &n11, &n20, &n21 }, level - 2),
		&NextGeneration(Node { &n11, &n12, &n21, &n22 }, level - 2)
	});
	m_Cache[node] = &result;

	return result;
}