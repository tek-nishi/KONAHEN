
#pragma once

//
// ゲーム開始演出
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class GameStart : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  EaseArray<Vec2<float> > easeArray_;
  float time_;
  float alpha_;
  float scale_;

  WidgetsMap::WidgetPtr widget_;

public:
  GameStart(GameEnv& env, const WidgetsMap& widgets) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["gamestart"].get<picojson::object>()),
    time_(),
    alpha_(),
    scale_(),
    widget_(widgets.get("gamestart"))
  {
    DOUT << "GameStart()" << std::endl;
    EasingArayVec2(QUAD_OUT, params_["easing"].get<picojson::array>(), easeArray_);
    if (env_.demo)
    {
      time_ = 1000;
      // デモプレイの場合は即時終了
    }
    else
    {
      env_.sound->play("gamestart");
    }
  }

  ~GameStart()
  {
    DOUT << "~GameStart()" << std::endl;
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
      env_.task->sendMsgAll(MSG_GAME_START);
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
