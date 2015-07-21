
#pragma once

//
// 「終了～！」演出
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "co_easearray.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"

namespace ngs {

class GameOver : public TaskProc {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  EaseArray<Vec2<float> > easeArray_;
  float time_;
  float alpha_;
  float scale_;

  const TexMng::tex_ptr texture_;
  WidgetsMap::WidgetPtr widget_;

public:
  GameOver(GameEnv& env, const WidgetsMap& widgets) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["gameover"].get<picojson::object>()),
    time_(),
    alpha_(),
    scale_(),
    widget_(widgets.get("gameover"))
  {
    DOUT << "GameOver()" << std::endl;
    EasingArayVec2(CUBIC_OUT, params_["easing"].get<picojson::array>(), easeArray_);
  }
  ~GameOver()
  {
    DOUT << "~GameOver()" << std::endl;
  }

  bool active() const { return active_; }

  void step(const float delta_time)
  {
    time_ += delta_time;

    Vec2<float> res;
    bool active = easeArray_.ease(res, time_);
    scale_ = res.x;
    alpha_ = res.y;

    if (!active)
    {
      active_ = false;
      env_.task->sendMsgAll(MSG_GAME_MENU_OFF);
      env_.task->sendMsgAll(MSG_GAME_GAMEOVER_END);
    }
  }
  
  void draw()
  {
    widget_->setScale(scale_, scale_);
    widget_->setCol(1.0, 1.0, 1.0, alpha_);
    widget_->draw();
  }

  void msg(const int msg) {}

};

}
