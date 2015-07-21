
#pragma once

//
// ベクトル
//

#include <cmath>

namespace ngs {

template <typename Type>
class Vec3
{
  Type _sin(Type a) const;
  Type _cos(Type a) const;
  Type _sqrt(Type a) const;
  Type _acos(Type a) const;
  // FIXME:intだけstd::sqrt()とかの呼び出しが曖昧になるための措置

public:
  Type x, y, z;

  Vec3() :
    x(),
    y(),
    z()
  {}

  Vec3(const Type xVal, const Type yVal, const Type zVal) :
    x(xVal),
    y(yVal),
    z(zVal)
  {}

  void set(const Type xVal, const Type yVal, const Type zVal) {
    x = xVal;
    y = yVal;
    z = zVal;
  }
    
  Type dist(const Vec3& src) const
  {
    Type dx(x - src.x);
    Type dy(y - src.y);
    Type dz(z - src.z);
    return _sqrt(dx * dx + dy * dy + dz *dz);
  }

  Type length() const
  {
    Type a = x * x + y * y + z * z;
    return this->_sqrt(a);
  }

  void scale(const Type s) {
    x = x * s;
    y = y * s;
    z = z * s;
  }

  bool unit() {
    Type l = this->length();
    if(l > static_cast<Type>(0))
    {
      x = x / l;
      y = y / l;
      z = z / l;
      return true;
    }
    else
    {
      x = y = z = static_cast<Type>(0);
      return false;
    }
  }

  Type dot(const Vec3& src) const
  {
    return x * src.x + y * src.y + z * src.z;
  }

  Vec3 cross(const Vec3& src) const
  {
    return Vec3(y * src.z - z * src.y, z * src.x - x * src.z, x * src.y - y * src.x);
  }

  Type angle(const Vec3& src) const
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
      
    return r;
  }

  Vec3 operator-() const
  {
    return Vec3(-x, -y, -z);
  }

  Vec3 operator+(const Vec3& vec) const
  {
    return Vec3(x + vec.x, y + vec.y, z + vec.z);
  }

  Vec3& operator+=(const Vec3& vec)
  {
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
  }

  Vec3 operator-(const Vec3& vec) const
  {
    return Vec3(x - vec.x, y - vec.y, z - vec.z);
  }

  Vec3& operator-=(const Vec3& vec)
  {
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
  }

  Vec3 operator*(const Type s) const
  {
    return Vec3(x * s, y * s, z * s);
  }

  Vec3& operator*=(const Type s)
  {
    this->scale(s);
    return *this;
  }

  Vec3 operator/(const Type s) const
  {
    return Vec3(x / s, y / s, z / s);
  }

  Vec3& operator/=(const Type s)
  {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }
};

// intだけstd::関数の呼び出しが曖昧になるための措置
template<typename Type> Type Vec3<Type>::_sin(Type a) const {
  return std::sin(a);
}
template<> int Vec3<int>::_sin(int a) const {
  return static_cast<int>(std::sin(static_cast<float>(a)));
}

template<typename Type> Type Vec3<Type>::_cos(Type a) const {
  return std::cos(a);
}
template<> int Vec3<int>::_cos(int a) const {
  return static_cast<int>(std::cos(static_cast<float>(a)));
}

template<typename Type> Type Vec3<Type>::_sqrt(Type a) const {
  return std::sqrt(a);
}
template<> int Vec3<int>::_sqrt(int a) const {
  return static_cast<int>(std::sqrt(static_cast<float>(a)));
}

template<typename Type> Type Vec3<Type>::_acos(Type a) const {
  return std::acos(a);
}
template<> int Vec3<int>::_acos(int a) const {
  return static_cast<int>(std::acos(static_cast<float>(a)));
}
  
}

