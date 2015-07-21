
#pragma once

//
// 光源のフェード処理
//

#include "nn_gameenv.hpp"


namespace ngs {

class FadeLight : public TaskProc {
  GameEnv& env_;
  bool active_;

  Earth& earth_;
  
  int type_;
  float scale_;
  float scale_st_;
  float scale_ed_;
  float time_cur_;
  float time_ed_;

  float delay_;

public:
  explicit FadeLight(GameEnv& env) :
    env_(env),
    active_(true),
    earth_(*env.earth),
    type_(LINEAR),
    scale_st_(earth_.lightScale()),
    scale_ed_(),
    time_cur_(),
    time_ed_(1.0),
    delay_()
  {
    DOUT << "FadeLight()" << std::endl;
    earth_.setLightScale(scale_st_);
  }
  ~FadeLight()
  {
    DOUT << "~FadeLight()" << std::endl;
  }

  bool active() const { return active_; }

  void type(int type) { type_ = type; }
  void time(const float time) { time_ed_ = time; }
  void scale(const float scale) { scale_ed_ = scale; }
  void delay(const float delay) { delay_ = delay; }
  
  void step(const float delta_time)
  {
    if (delay_ > 0.0)
    {
      delay_ -= delta_time;
      if (delay_ > 0.0) return;
    }

    time_cur_ += delta_time;

    Easing easing;
    easing.ease(scale_, time_cur_, scale_st_, scale_ed_, time_ed_, type_);
    earth_.setLightScale(scale_);

    bool active = (time_cur_ < time_ed_);
    if (!active) active_ = false;
  }

  void draw() {}

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_TITLE_INTRO_SKIP:
      {
        delay_ = 0;
      }
      break;
      
    case MSG_FADELIGHT_STOP:
      {
        earth_.setLightScale(scale_ed_);
        active_ = false;
      }
      break;
    }
  }
};

}
