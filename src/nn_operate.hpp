
#pragma once

//
// DEMO画面での操作表示
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"
#include "nn_opedevice_mouse.hpp"
#include "nn_opedevice_touch.hpp"


namespace ngs {

class Operate : public TaskProc {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;
  OpeDevice device_;

  Quat<float> rot_;
  float dist_;

public:
  explicit Operate(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
    device_(env),
    rot_(env_.camera->getRot()),
    dist_(env.camera->getDist())
  {
    DOUT << "Operate()" << std::endl;
  }

  ~Operate() {
    DOUT << "~Operate()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    Quat<float> rot = env_.camera->getRot();

    if (rot_ != rot)
    {
      float dot = rot_.x * rot.x + rot_.y * rot.y + rot_.z * rot.z + rot_.w * rot.w;
      dot = minmax(dot, -1.0f, 1.0f);
      if (dot < 0.0) dot = -dot;
      float r = std::acos(dot);

      if (r > ((PI * 0.05) * delta_time))
      {
        device_.dragLeft();
      }
      rot_ = rot;
    }
    
    float dist = env_.camera->getDist();
    if (dist_ != dist)
    {
      dist_ = dist;
      device_.dragRight();
    }
    device_.step(delta_time);
  }
  
  void draw()
  {
    device_.draw();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_DEMOPLAY_END:
      {
        active_ = false;
      }
      break;

    case MSG_GAME_TOUCH:
      {
        device_.touchLeft();
      }
      break;
    }
  }

};

}
