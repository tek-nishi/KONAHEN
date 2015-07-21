
#pragma once

//
// メインループ基底クラス
//

namespace ngs {

class ProcBase {
public:
  ProcBase() {}
  virtual ~ProcBase() {}

  virtual void resize(const int w, const int h) = 0;
  virtual void resize(const int w, const int h, const float sx, const float sy) = 0;
  virtual void y_bottom(const float y) = 0;
  virtual bool step(const float delta_time) = 0;
  virtual void draw() = 0;

#ifdef _DEBUG
  virtual void forceFrame(const bool force) = 0;
#endif
};

}
