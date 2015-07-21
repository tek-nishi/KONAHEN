
#pragma once

//
// ゲーム用のタイマー
//

#if defined (_MSC_VER)

#include <deque>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include "co_avarage.hpp"


namespace ngs {

class GameTimer
{
  enum { REC_MAX = 30 };

  double time_;
  Avarage<double> times_;
  float last_;
  float avg_;
public:
  GameTimer() :
    time_(timeGetTime()),
    times_(0, REC_MAX),
    last_(),
    avg_()
  {}
  ~GameTimer() {}

  void update()
  {
    double t = timeGetTime();
    double dt = t - time_;
    last_ = dt / 1000.0;
    avg_ = (times_ += dt) / 1000.0;
    time_ = t;
  }

  float avarage() const { return avg_; }
  float last() const { return last_; }
};

}

#endif
