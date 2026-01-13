#include "Graphics2D.h"
#include "LifeAlgorithm.h"
#include "LifeHashSet.h"

gol::LifeHashSet gol::SparseLife(const LifeHashSet& data, const Rect& bounds)
{
	constexpr static int32_t dx[] = { -1,-1,-1,0,0,1,1,1 };
	constexpr static int32_t dy[] = { -1,0,1,-1,1,-1,0,1 };

	ankerl::unordered_dense::map<Vec2, uint8_t> neighborCount;
	neighborCount.reserve(data.size() * 8);
	for (auto&& pos : data)
	{
		for (int32_t i = 0; i < 8; ++i)
		{
			int32_t x = pos.X + dx[i];
			int32_t y = pos.Y + dy[i];
			if (bounds.Width > 0 && bounds.Height > 0 && !bounds.InBounds(x, y))
				continue;

			++neighborCount[{x, y}];
		}
	}

	LifeHashSet newSet{};
	newSet.reserve(neighborCount.size());
	for (auto&& [pos, neighbors] : neighborCount)
	{
		if (neighbors == 3 || (neighbors == 2 && data.contains(pos)))
			newSet.insert(pos);
	}
	return newSet;
}