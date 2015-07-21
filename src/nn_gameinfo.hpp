
#pragma once

//
// 各種進行情報の表示
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

class GameInfo : public TaskProc {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  const WidgetsMap& widgets_;
  WidgetsMap::WidgetPtr widget_;

  float delay_;
  float time_;
  Vec2<float> ofs_;
  float alpha_;
  EaseArray<Vec2<float> > ease_;

  std::string req_;

  bool draw_;

  void setup(const std::string& name)
  {
    widget_ = widgets_.get(name);
    time_ = 0;
    draw_ = true;
  }
  
public:
  GameInfo(GameEnv& env, const WidgetsMap& widgets) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["gameinfo"].get<picojson::object>()),
    widgets_(widgets),
    delay_(),
    time_(),
    alpha_(),
    draw_()
  {
    DOUT << "GameInfo()" << std::endl;
    EasingArayVec2(EXPO_OUT, params_["ease_in"].get<picojson::array>(), ease_);
    EasingArayVec2(EXPO_IN, params_["ease_out"].get<picojson::array>(), ease_, true);
  }
  ~GameInfo()
  {
    DOUT << "~GameInfo()" << std::endl;
  }

  bool active() const { return active_; }

  void step(const float delta_time)
  {
    if (delay_ > 0)
    {
      if ((delay_ -= delta_time) <= 0)
      {
        setup(req_);
      }
    }

    if (!draw_) return;
    
    time_ += delta_time;

    Vec2<float> res; 
    draw_ = ease_.ease(res, time_);
    ofs_.x = (env_.size->x / 2.0 + 250) * res.x;
    alpha_ = res.y;
  }

  void draw()
  {
    if (!draw_) return;
    
    const Vec2<float>& pos = widget_->posOrig();
    widget_->setPos(pos + ofs_);
    widget_->setCol(1, 1, 1, alpha_);
    widget_->draw();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_CLEANUP:
      {
        active_ = false;
      }
      break;
      
    case MSG_GAME_LEVELUP:
      {
        setup(std::string("levelup"));
      }
      break;

    case MSG_GAME_HURRYUP:
      {
        setup(std::string("hurryup"));
        env_.sound->play("alert");
      }
      break;

    case MSG_GAME_TIMESHORT:
      {
        setup(std::string("time_short"));
      }
      break;

    case MSG_GAME_DOBEST:
      {
        req_ = "dobest";
        delay_ = params_["delay"].get<double>();
      }
      break;

    case MSG_GAME_HINT:
      {
        setup(std::string("hint"));
      }
      break;
    }
  }
};

}
