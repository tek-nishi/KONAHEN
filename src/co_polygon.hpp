
#pragma once

#include <vector>
#include "co_vec2.hpp"
#include "co_graph.hpp"

namespace ngs {

template <typename T>
class Polygon {
  std::vector<Vec2<T> > vtx_;

public:
  void setVtx(const Vec2<T>& pos)
  {
    vtx_.push_back(pos);
  }
  void setVtx(const std::vector<Vec2<T> >& vtx)
  {
    vtx_ = vtx;
  }
  const std::vector<Vec2<T> >& vtx() const
  {
    return vtx_;
  }
  
  void clear()
  {
    vtx_.clear();
  }
  
  std::size_t numVtx() const
  {
    return vtx_.size();
  }

  // 点の内包判定
  bool intension(const Vec2<T>& pos) const
  {
    int cou = 0;
    std::size_t num = this->numVtx();
    for(std::size_t i = 0; i < num; ++i)
    {
      T x1, y1, x2, y2;

      x1 = vtx_[i].x;
      y1 = vtx_[i].y;

      // int idx = ((i + 1) != num) ? i + 1 : 0;
      // x2 = vtx_[idx].x;
      // y2 = vtx_[idx].y;
      x2 = vtx_[(i + 1) % num].x;
      y2 = vtx_[(i + 1) % num].y;

      if(y1 > y2)
      {
        std::swap(x1, x2);
        std::swap(y1, y2);
      }

      if(pos.y >= y1 && pos.y < y2)
      {
        // y = pos.y の位置でのスキャンライン上の交差位置を求める
        // SOURCE: 点(x1, y1)を通る傾きmの直線 y - y1 = m(x - x1)
        //         をxについて解いている
        T xx = (x2 - x1) * (pos.y - y1) / (y2 - y1) + x1;
        if(xx < pos.x) ++cou;
      }
    }
    return (cou & 1);
  }

  // 円の接触判定
  bool touch(const Vec2<T>& pos, const float radius) const
  {
    std::size_t num = this->numVtx();
    std::size_t i;
    for(i = 0; i < num; ++i)
    {
      const Vec2<float>& st = vtx_[i];
      const Vec2<float>& ed = vtx_[(i + 1) % num];
      
      if (crossCircleLine(st, ed, pos, radius)) break;
    }
    return i < num;
  }

  Vec2<T> center() const
  {
    Vec2<T> res;
    for (typename std::vector<Vec2<T> >::const_iterator it = vtx_.begin(); it != vtx_.end(); ++it)
    {
      res += *it;
    }
    // FIXME:より正確に多角形の中心を求める手法

    std::size_t num = this->numVtx();
    res.x /= num;
    res.y /= num;

    return res;
  }

  void draw() const
  {
    std::size_t num = vtx_.size();
    for(std::size_t i = 0; i < num; ++i)
    {
      int idx = ((i + 1) != num) ? i + 1 : 0;
      GrpLine obj;
      obj.points(vtx_[i].x, vtx_[i].y, vtx_[idx].x, vtx_[idx].y);
      obj.col(1.0, 0.0, 0.0);
      obj.draw();
    }

    {
      GrpPoint obj;
      obj.pos(this->center());
      obj.col(1.0, 1.0, 0.0);
      obj.size(2);
      obj.draw();
    }
  }
  
};

}
