
#pragma once

//
// ゲーム用のタイマー
//

#if defined (__APPLE__) || defined (__linux__)

#include <deque>
#include <sys/time.h>
#include "co_avarage.hpp"


namespace ngs {

double getCurTime()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.001 * 0.001;
}


class GameTimer
{
	enum { REC_MAX = 30 };

	double time_;
	Avarage<double> times_;
	float last_;
	float avg_;
public:
	GameTimer() :
		time_(getCurTime()),
		times_(0, REC_MAX),
		last_(),
		avg_()
	{}
	~GameTimer() {}

	void update()
	{
		double t = getCurTime();
		double dt = t - time_;
		last_ = dt;
		avg_ = (times_ += dt);
		time_ = t;
	}

	float avarage() const { return avg_; }
	float last() const { return last_; }
};

}

#endif
