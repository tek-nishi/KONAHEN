
#pragma once

//
// クオータニオン
// ※型はfloatもしくはdoubleで使ってください
//

#include <cfloat>
#include <cmath>
#include "co_vec3.hpp"

namespace ngs {

template <typename T>
class Quat {
  T _sqrt(T a) const;

public:
  T x, y, z, w;

  Quat() :
    x(),
    y(),
    z(),
    w(static_cast<T>(1))
  {}

  Quat(const T xVal, const T yVal, const T zVal, const T wVal) :
    x(xVal),
    y(yVal),
    z(zVal),
    w(wVal)
  {}

  void set(const T xVal, const T yVal, const T zVal, const T wVal)
  {
    x = xVal;
    y = yVal;
    z = zVal;
    w = wVal;
  }

  void rotate(const T r, const Vec3<T>& vct)
  {
    T sin_r = std::sin(r / static_cast<T>(2));

    x = vct.x * sin_r;
    y = vct.y * sin_r;
    z = vct.z * sin_r;
    w = std::cos(r / static_cast<T>(2));
  }

  T length() const
  {
    T a = x * x + y * y + z * z + w * w;
    return this->_sqrt(a);
  }

  bool unit()
  {
    T l = this->length();
    if(l > static_cast<T>(0))
    {
      x = x / l;
      y = y / l;
      z = z / l;
      w = w / l;
      return true;
    }
    else
    {
      x = y = z = static_cast<T>(0);
      w = static_cast<T>(1);
      return false;
    }
  }

  Quat operator*(const Quat& src) const
  {
    T x_ = w * src.x + x * src.w + y * src.z - z * src.y;
    T y_ = w * src.y - x * src.z + y * src.w + z * src.x;
    T z_ = w * src.z + x * src.y - y * src.x + z * src.w;
    T w_ = w * src.w - x * src.x - y * src.y - z * src.z;
    return Quat(x_, y_, z_, w_);
  }

  Quat& operator*=(const Quat& src)
  {
    T x_ = w * src.x + x * src.w + y * src.z - z * src.y;
    T y_ = w * src.y - x * src.z + y * src.w + z * src.x;
    T z_ = w * src.z + x * src.y - y * src.x + z * src.w;
    T w_ = w * src.w - x * src.x - y * src.y - z * src.z;

    x = x_;
    y = y_;
    z = z_;
    w = w_;

    return *this;
  }

  bool operator==(const Quat& src)
  {
    return (x == src.x) && (y == src.y) && (z == src.z) && (w == src.w);
  }
  bool operator!=(const Quat& src)
  {
    return (x != src.x) || (y != src.y) || (z != src.z) || (w != src.w);
  }
};

template<typename T> T Quat<T>::_sqrt(T a) const
{
  return std::sqrt(a);
}

template<> int Quat<int>::_sqrt(int a) const
{
  return static_cast<int>(std::sqrt(static_cast<double>(a)));
}


template<typename T>
Quat<T> slerp(const Quat<T>& q, const Quat<T>& r, const T t)
{
  T dot = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;
  dot = minmax(dot, static_cast<T>(-1), static_cast<T>(1));
  bool negative = false;
  if (dot < static_cast<T>(0))
  {
    negative = true;
    dot = -dot;
    // TIPS:内積が負の値の場合に遠回りの補間になってしまうのを防ぐ
  }

  T a = std::acos(dot);
  T s = std::sin(a);
  if (s < static_cast<T>(FLT_EPSILON))
  {
    return r;
  }
  else
  {
    T sq = std::sin((static_cast<T>(1) - t) * a);
    T sr = std::sin(t * a);
    sr = negative ? -sr : sr;

    return Quat<T>((q.x * sq + r.x * sr) / s,
                   (q.y * sq + r.y * sr) / s,
                   (q.z * sq + r.z * sr) / s,
                   (q.w * sq + r.w * sr) / s);
  }
}

}

