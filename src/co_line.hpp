
#pragma once

//
// 直線
// 

#include "co_vec2.hpp"

namespace ngs {

	template <typename Type>
	class Line {
	public:
		Vec2<Type> start;
		Vec2<Type> end;

		void set(const Vec2<Type>& st, const Vec2<Type>& ed) {
			start = st;
			end = ed;
		}
	};
	
}
