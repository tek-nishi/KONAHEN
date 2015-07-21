
#pragma once

//
// ベクトル
//

namespace ngs {

template <typename Type>
class Vec4 {
public:
  Type x, y, z, w;

  Vec4() : x(), y(), z(), w() {}
  Vec4(Type xVal, Type yVal, Type zVal, Type wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}

  void set(Type xVal, Type yVal, Type zVal, Type wVal) {
    x = xVal;
    y = yVal;
    z = zVal;
    w = wVal;
  }

  void scale(const Type s) {
    x = x * s;
    y = y * s;
    z = z * s;
    w = w * s;
  }

  Vec4 operator-() const {
    return Vec4(-x, -y, -z, -w);
  }

  Vec4 operator+(const Vec4& vec) const {
    return Vec4(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
  }

  Vec4& operator+=(const Vec4& vec) {
    x += vec.x;
    y += vec.y;
    z += vec.z;
    w += vec.w;
    return *this;
  }

  Vec4 operator-(const Vec4& vec) const {
    return Vec4(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
  }

  Vec4& operator-=(const Vec4& vec) {
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    w -= vec.w;
    return *this;
  }

  Vec4 operator*(const Type s) const {
    return Vec4(x * s, y * s, z * s, w * s);
  }

  Vec4& operator*=(const Type s) {
    this->scale(s);
    return *this;
  }

  Vec4 operator/(const Type s) const {
    return Vec4(x / s, y / s, z / s, w / s);
  }

  Vec4& operator/=(const Type s) {
    x /= s;
    y /= s;
    z /= s;
    w /= s;
    return *this;
  }
};

}

