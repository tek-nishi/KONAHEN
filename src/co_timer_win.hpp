
#pragma once

//
// 処理時間計測
//

#if defined (_MSC_VER)

#include <windows.h>

namespace ngs {

class Timer {
	DWORD counter_;
	DWORD last_;
	
public:
	Timer() :
		counter_(timeGetTime()),
		last_(counter_)
	{}
	~Timer() {}

	double get() const
	{
		DWORD dt = timeGetTime() - counter_;
		if (!dt) dt = 1;
		return dt / 1000.0;
	}

	double last()
	{
		DWORD cur = timeGetTime();
		DWORD dt = cur - last_;
		if (!dt) dt = 1;
		last_ = cur;
		return dt / 1000.0;
	}
};

}

#endif
