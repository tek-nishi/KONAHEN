
#pragma once

//
// ゲーム中の3D空間表示
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_earth.hpp"
#include "nn_space.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class GameWorld : public TaskProc {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  Camera& camera_;

  Earth& earth_;
  Light& earthLight_;                               // FIXME:わざわざ参照にして持たなくても…
  Space space_;

  float cloud_alpha_;
  float time_, time_ed_;
  float cloud_alpha_st_, cloud_alpha_ed_;

  const float cloud_dist_max_;
  const float cloud_dist_min_;
  const float cloud_alpha_max_;
  const float cloud_alpha_min_;

public:
  explicit GameWorld(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()),
    camera_(*env.camera),
    earth_(*env.earth),
    earthLight_(*env.earthLight),
    space_(params_, camera_),
    cloud_alpha_(1),
    time_(),
    time_ed_(),
    cloud_alpha_st_(),
    cloud_alpha_ed_(),
    cloud_dist_max_(params_["cloud_dist_max"].get<double>()),
    cloud_dist_min_(params_["cloud_dist_min"].get<double>()),
    cloud_alpha_max_(params_["cloud_alpha_max"].get<double>()),
    cloud_alpha_min_(params_["cloud_alpha_min"].get<double>())
  {
    DOUT << "GameWorld()" << std::endl;
  }
  ~GameWorld()
  {
    DOUT << "~GameWorld()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    space_.step(delta_time);
    earth_.step(delta_time);
    if (time_ < time_ed_)
    {
      time_ += delta_time;

      Easing easing;
      easing.ease(cloud_alpha_, time_, cloud_alpha_st_, cloud_alpha_ed_, time_ed_);
    }
  }

  void draw()
  {
    float dist = camera_.getDist();
    float scale = 1.0;
    if (dist < (cloud_dist_max_ * cloud_alpha_))
    {
      scale = (dist - cloud_dist_min_) / ((cloud_dist_max_ * cloud_alpha_) - cloud_dist_min_);
      if (scale < 0) scale = 0;
    }
    earth_.cloud_alpha(cloud_alpha_min_ + (cloud_alpha_max_ - cloud_alpha_min_) * scale);
    // カメラが地球に寄ったら雲を消す

    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    // glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    // glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    camera_.setup();
    space_.draw();
    
    const Matrix& mtx = camera_.getMatrix();
    Vec3<float> lpos = earthLight_.getOfs();
    mtx.apply(lpos);
    earthLight_.pos(lpos);
    
    earth_.draw();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_MAIN_INIT:
      {
        if (env_.game_mode == GAME_MODE_NORMAL)
        {
          cloud_alpha_st_ = cloud_alpha_;
          cloud_alpha_ed_ = 1.5;
          time_ = 0;
          time_ed_ = 1.5;
        }
      }
      break;

//    case MSG_GAME_LEVELUP:
    case MSG_GAME_CLEANUP:
      {
        cloud_alpha_st_ = cloud_alpha_;
        cloud_alpha_ed_ = 1.0;
        time_ = 0;
        time_ed_ = 1.5;
      }
      break;
    }
  }
};

}
