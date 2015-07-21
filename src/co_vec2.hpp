
#pragma once

//
// ベクトル
//

#include <cmath>

namespace ngs {

template <typename Type>
class Vec2 {
  Type _sin(Type a) const;
  Type _cos(Type a) const;
  Type _sqrt(Type a) const;
  Type _acos(Type a) const;
  // FIXME:intだけstd::sqrt()とかの呼び出しが曖昧になるための措置

public:
  Type x, y;

  Vec2() :
    x(),
    y()
  {}
  
  Vec2(const Type xVal, const Type yVal) :
    x(xVal),
    y(yVal)
  {}
  
  Vec2(const Vec2<Type>& src) :
    x(src.x),
    y(src.y)
  {}

  void set(const Type xVal, const Type yVal)
  {
    x = xVal;
    y = yVal;
  }
    
  Type dist(const Vec2& src) const {
    Type dx(x - src.x);
    Type dy(y - src.y);
    return this->_sqrt(dx * dx + dy * dy);
  }

  Type length() const {
    Type a = x * x + y * y;
    return this->_sqrt(a);
  }

  void scale(const Type s) {
    x = x * s;
    y = y * s;
  }

  bool unit() {
    Type l = this->length();
    if(l > static_cast<Type>(0))
    {
      x = x / l;
      y = y / l;
      return true;
    }
    else
    {
      x = y = static_cast<Type>(0);
      return false;
    }
  }

  Type dot(const Vec2& src) const {
    return x * src.x + y * src.y;
  }

  Type cross(const Vec2& src) const {
    return x * src.y - src.x * y;
  }

  void rotate(const Type r) {
    Type sin_r = this->_sin(r);
    Type cos_r = this->_cos(r);

    Type _x = x;
    Type _y = y;
    x = _x * cos_r - _y * sin_r;
    y = _x * sin_r + _y * cos_r;
  }

  Type angle(const Vec2& src) const
  {
    Type l1 = this->length();
    Type l2 = src.length();
    if((l1 <= static_cast<Type>(0)) || (l2 <= static_cast<Type>(0)))
    {
      return static_cast<Type>(0);
      // どちらかのベクトルの長さがゼロの場合、角度は０
    }
      
    Type value = this->dot(src) / (l1 * l2);
    Type r = static_cast<Type>(0);
    if(value < static_cast<Type>(1)) r = this->_acos(value);
    // FIXME:外積を使ってマイナスの値を決める必要がある
      
    return r;
  }

  Type angleY() const {
    Vec2 src(0, -1);
    Type r = this->angle(src);
    if(x < static_cast<Type>(0)) r = -r;

    return r;
  }

  Type angleX() const {
    Vec2 src(1, 0);
    Type r = this->angle(src);
    if(y < static_cast<Type>(0)) r = -r;

    return r;
  }

  Vec2 normal() const {
    Vec2 dst(-y, x);
    dst.unit();
    return dst;
  }

  Vec2 refrect(const Vec2& src) const {
    Vec2 n = this->normal();
    float d = n.dot(src) * static_cast<Type>(2);
    float rx = src.x - d * n.x;
    float ry = src.y - d * n.y;
    Vec2 res(rx, ry);
    return res;
  }

  Vec2 parallel(const Vec2& src) const {
    Vec2 n = this->normal();
    float d = n.dot(src);
    float rx = src.x - d * n.x;
    float ry = src.y - d * n.y;
    Vec2 res(rx, ry);
    return res;
  }

  Vec2 operator-() const {
    return Vec2(-x, -y);
  }

  Vec2 operator+(const Vec2& vec) const {
    return Vec2(x + vec.x, y + vec.y);
  }

  Vec2& operator+=(const Vec2& vec) {
    x += vec.x;
    y += vec.y;
    return *this;
  }

  Vec2 operator-(const Vec2& vec) const {
    return Vec2(x - vec.x, y - vec.y);
  }

  Vec2& operator-=(const Vec2& vec) {
    x -= vec.x;
    y -= vec.y;
    return *this;
  }

  Vec2 operator*(const Type s) const {
    return Vec2(x * s, y * s);
  }

  Vec2& operator*=(const Type s) {
    scale(s);
    return *this;
  }

  Vec2 operator/(const Type s) const {
    return Vec2(x / s, y / s);
  }

  Vec2& operator/=(const Type s) {
    x /= s;
    y /= s;
    return *this;
  }
};

// intだけstd::関数の呼び出しが曖昧になるための措置
template<typename Type> Type Vec2<Type>::_sin(Type a) const {
  return std::sin(a);
}
template<> int Vec2<int>::_sin(int a) const {
  return std::sin(static_cast<float>(a));
}

template<typename Type> Type Vec2<Type>::_cos(Type a) const {
  return std::cos(a);
}
template<> int Vec2<int>::_cos(int a) const {
  return std::cos(static_cast<float>(a));
}

template<typename Type> Type Vec2<Type>::_sqrt(Type a) const {
  return std::sqrt(a);
}
template<> int Vec2<int>::_sqrt(int a) const {
  return std::sqrt(static_cast<float>(a));
}

template<typename Type> Type Vec2<Type>::_acos(Type a) const {
  return std::acos(a);
}
template<> int Vec2<int>::_acos(int a) const {
  return std::acos(static_cast<float>(a));
}
  
}

