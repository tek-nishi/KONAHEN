
#pragma once

//
// タイマー計測などに使う簡単な平均化
//

#include <deque>

namespace ngs {

template <typename T>
class Avarage
{
  enum { AVG_MAX = 10 };
  T num_;

  std::deque<T> values_;
  T t_;
  T avg_;

public:
  Avarage(const T value, const int num = AVG_MAX) :
    num_(num),
    values_(num, value),
    t_()
  {}

  T get() const { return avg_; }

  T operator+=(const T value)
  {
    T last = values_[0];
    values_.pop_front();
    values_.push_back(value);
    t_ = t_ - last + value;
    avg_ = (t_ / num_);
    return avg_;
  }
  
};

}

