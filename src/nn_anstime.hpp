
#pragma once

//
// 解答時間表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class AnsTime : public TaskProc {
  GameEnv& env_;
  bool active_;

  WidgetsMap::WidgetPtr widget_;
  const TexMng::tex_ptr texture_;

  bool started_;
  bool refresh_;
  float angle_;

  float time_, time_ed_;
  Vec2<float> ofs_;
  Vec2<float> ofs_st_, ofs_ed_;

  float ang_time_;
  float angle_st_;
  float ang_time_ed_;
  
  bool inout_;
  bool exec_fin_;

public:
  AnsTime(GameEnv& env, const WidgetsMap& widgets) :
    env_(env),
    active_(true),
    widget_(widgets.get("ans_time")),
    texture_(env.texMng->read(*env.path + "devdata/game.png")),
    started_(),
    refresh_(),
    angle_(),
    time_(),
    time_ed_(1.0),
    ofs_st_(200, 0),
    ofs_ed_(0, 1),
    ang_time_(),
    angle_st_(angle_),
    ang_time_ed_(2.0),
    inout_(true),
    exec_fin_()
  {
    DOUT << "AnsTime()" << std::endl;
  }
  ~AnsTime()
  {
    DOUT << "~AnsTime()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (inout_)
    {
      time_ += delta_time;

      Easing easing;
      easing.ease(ofs_, time_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);
      inout_ = (time_ < time_ed_);
      if (exec_fin_ && !inout_) active_ = false;
    }

    if (!refresh_ && (ang_time_ed_ > 0.0))
    {
      ang_time_ += delta_time;
      Easing easing;
      easing.ease(angle_, ang_time_, angle_st_, PI * 2.0f, ang_time_ed_, CUBIC_OUT);
    }
  }

  void draw()
  {
    if (refresh_ && (env_.ans_time_max > 0.0))
    {
      float a = env_.ans_time / env_.ans_time_max;
      if (a < 0.0) a = 0.0f;
      angle_ = a * PI * 2.0f;
    }

    GrpCol<float> col(1, 1, 1, ofs_.y);
    if (started_ && (angle_ < (PI / 2.0))) col.set(1, 0, 0, ofs_.y);

    Vec2<float> pos = widget_->posOrig();
    widget_->setPos(pos.x + ofs_.x, pos.y);
    widget_->setCol(col);
    widget_->draw();

    if (angle_ > 0.0f)
    {
      const Vec2<float>& pos = widget_->dispPos();
      int div = (32.0 * angle_) / (PI * 2.0);
      if (div < 2) div = 2;

      GrpFan obj;
      obj.pos(pos.x + 46.5, pos.y + 47);
      obj.radius(30, 30);
      obj.div(div);
      obj.angle(angle_);
      obj.fill(true);
      obj.smooth(true);
      obj.rotate(PI);
      obj.col(col);
      obj.texture(texture_);
      obj.uv(468, 6, 468 + 40, 6 + 2);
      obj.draw();
    }
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

    case MSG_GAME_MENU_OFF:
      {
        inout_ = true;
        time_ = 0;
        ofs_st_ = ofs_;
        ofs_ed_.set(200, 0);
        exec_fin_ = true;
      }
      break;

    case MSG_GAME_TOUCH_START:
      {
        started_ = true;
        refresh_ = true;
      }
      break;

    case MSG_GAME_TOUCH_ANS:
      {
        ang_time_ = 0.0;
        angle_st_ = angle_;
        ang_time_ed_ = 1.0;
        refresh_ = false;
      }
      break;
      
    case MSG_GAME_TOUCH_IN:
      {
        ang_time_ = 0.0;
        angle_st_ = angle_;
        ang_time_ed_ = 0.5;
        refresh_ = false;
      }
      break;

    case MSG_GAME_END:
      {
        ang_time_ed_ = 0.0;
        refresh_ = false;
      }
      break;
    }
  }
  
};

}
