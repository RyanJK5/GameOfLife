#ifndef __LifeAlgorithm_h__
#define __LifeAlgorithm_h__

#include <functional>

#include "Graphics2D.h"
#include "LifeHashSet.h"

namespace gol
{
	using LifeAlgorithm = std::function<LifeHashSet(const LifeHashSet&, const Rect&)>;

	LifeHashSet SparseLife(const LifeHashSet& data, const Rect& bounds);

	LifeHashSet HashLife(const LifeHashSet& data, const Rect& bounds);
}

#endif