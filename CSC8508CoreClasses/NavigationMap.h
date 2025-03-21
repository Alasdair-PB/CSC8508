#pragma once
#include "NavigationPath.h"
namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8508 {
		class NavigationMap
		{
		public:
			NavigationMap() {}
			~NavigationMap() {}

			virtual bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) = 0;
		};
	}
}

