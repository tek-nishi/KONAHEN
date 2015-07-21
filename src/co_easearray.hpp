
#pragma once

//
// イージング配列
//

#include <vector>
#include "co_easing.hpp"


namespace ngs {

template <typename T>
class EaseArray {
	struct EaseInfo {
		int type;
		T st, ed;
		float d;
	};
	std::vector<EaseInfo> array_;
	float d_;

	Easing easing_;

public:
	EaseArray() :
		d_()
	{}

	void push_back(int type, T st, T ed, float d)
	{
		EaseInfo info = { type, st, ed, d };
		array_.push_back(info);
		d_ += d;
	}

	void clear() {
		d_ = 0.0;
		array_.clear();
	}

	bool ease(T& res, const float t)
	{
		float time = 0.0 < t ? (t < d_ ? t : d_) : 0.0;
		float t_st = 0.0;
		for (typename std::vector<EaseInfo>::iterator it = array_.begin(); it != array_.end(); ++it)
		{
			float t_ed = t_st + it->d;
			if(time <= t_ed)
			{
				easing_.ease(res, time - t_st, it->st, it->ed, it->d, it->type);
				break;
			}
			t_st = t_ed;
		}

		return t <= d_;
	}

	float duration() const { return d_; }
	
};

}
