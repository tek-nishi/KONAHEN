
#pragma once

//
// イージングプレビュワー
//

#if !(TARGET_OS_IPHONE) && _DEBUG

#include <string>
#include "co_keyinp.hpp"
#include "co_easing.hpp"
#include "co_font.hpp"
#include "nn_camera.hpp"
#include "nn_proc_base.hpp"


namespace ngs {


void drawEasing(const int type, const float x, const float y)
{
  Easing easing;
  
  glPushMatrix();
  glTranslatef(x, y, 0);
  glScalef(100, 100, 1.0);
  {
    GrpLine obj;
    obj.points(0, 0, 1, 0);
    obj.col(1, 1, 1);
    obj.draw();

    obj.points(0, 0, 0, -1);
    obj.draw();

    obj.col(1, 0, 0);

    float v1;
    easing.ease(v1, 0, 0, 1, 1, type);
    for(int i = 1; i <= 50; ++i)
    {
      float v2;
      easing.ease(v2, i / 50.0, 0, 1, 1, type);
      obj.points((i - 1) / 50.0, -v1, i / 50.0, -v2);
      obj.draw();
      v1 = v2;
    }
  }
  glPopMatrix();
}


class EasingTest : public ProcBase
{
  Keyinp& keyinp_;
  Vec2<int> size_;
  Camera cockpit_;

  Font font_;

public:
  EasingTest(const Vec2<int>& size, const float scale, Touch& touch, Keyinp& keyinp, const std::string& path) :
    keyinp_(keyinp),
    size_(size),
    cockpit_(Camera::ORTHOGONAL),
    font_(FONT_TEXTURE, path + "devdata/VeraMono.ttf", 12)
  {
    DOUT << "EasingTest()" << std::endl;
    cockpit_.setSize(size.x, size.y);
  }
  ~EasingTest() {
    DOUT << "~EasingTest()" << std::endl;
  }

  void resize(const int w, const int h) {}
  void resize(const int w, const int h, const float sx, const float sy) {
    cockpit_.setSize(w, h);
  }
  void y_bottom(const float y) {}
  
  bool step(const float delta_time) { return true; }

  void draw() {
    static const char *name[] = {
      "Linear",

      "BackIn",
      "BackOut",
      "BackInOut",

      "BounceIn",
      "BounceOut",
      "BounceInOut",

      "CircIn",
      "CircOut",
      "CircInOut",

      "CubicIn",
      "CubicOut",
      "CubicInOut",

      "ElasticIn",
      "ElasticOut",
      "ElasticInOut",

      "ExpoIn",
      "ExpoOut",
      "ExpoInOut",

      "QuadIn",
      "QuadOut",
      "QuadInOut",

      "QuartIn",
      "QuartOut",
      "QuartInOut",

      "QuintIn",
      "QuintOut",
      "QuintInOut",

      "SineIn",
      "SineOut",
      "SineInOut",
    };
    
    cockpit_.setup();
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    for(int i = LINEAR; i <= SINE_INOUT; ++i)
    {
      float x = (i % 7) * 110.0 - 400;
      float y = i / 7 * 125.0 - 200;

      glPushMatrix();
      glTranslatef(x, y + 12, 0);
      font_.draw(std::string(name[i]));
      glPopMatrix();

      drawEasing(i, x, y);
    }
  }

#ifdef _DEBUG
  virtual void forceFrame(const bool force) {}
#endif
};
    
}

#endif
