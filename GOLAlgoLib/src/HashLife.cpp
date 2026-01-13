#include <memory>
#include <unordered_dense.h>


#include "LifeAlgorithm.h"

namespace gol {
	namespace {
		struct LifeNode {
			LifeNode* NorthWest;
			LifeNode* NorthEast;
			LifeNode* SouthWest;
			LifeNode* SouthEast;
		};
		
		const LifeNode *const TrueNode = new LifeNode { nullptr, nullptr, nullptr, nullptr };
		const LifeNode *const FalseNode = nullptr;
		
		ankerl::unordered_dense::map<LifeNode*, LifeNode*> HashLifeMap {};
		std::vector<std::unique_ptr<LifeNode>> NodeStorage {};
	}

	
	LifeHashSet HashLife(const LifeHashSet& data, const Rect& bounds)
	{
		return data;
	}
}
