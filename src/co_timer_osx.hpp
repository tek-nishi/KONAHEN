
#pragma once

//
// 処理時間計測
//

#if defined (__APPLE__) || defined (__linux__)

#include <sys/time.h>

namespace ngs {

class Timer {
	timeval counter_;
	timeval last_;

	double getTime(const timeval& tv) const
	{
		return (double)tv.tv_sec + (double)tv.tv_usec * 0.001 * 0.001;
	}
	
public:
	Timer()
	{
		gettimeofday(&counter_, NULL);
		last_ = counter_;
	}
	~Timer()
	{
	}

	double get() const
	{
		timeval current;
		gettimeofday(&current, NULL);
		
		return this->getTime(current) - this->getTime(counter_);
	}

	double last()
	{
		timeval current;
		gettimeofday(&current, NULL);
		double dt = this->getTime(current) - this->getTime(last_);
		last_ = current;

		return dt;
	}
};

}

#endif
