
#pragma once

//
// 地球のテクスチャ変更
//

#include <iostream>
#include <string>
#include "co_audio.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class EarthTexture : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  WidgetsMap::WidgetPtr widget_;
  std::vector<WidgetsMap::WidgetPtr> btn_widgets_;

  int texture_index_;
  float chg_delay_;

  float delay_;
  bool inout_;
  bool menu_open_;
  bool btn_disp_;
  float col_;

  float menu_width_;

  int ease_mode_;
  float time_cur_, time_ed_;
  Vec3<float> vec_, vec_st_, vec_ed_;
  float btn_alpha_, btn_alpha_st_, btn_alpha_ed_; 

  bool draw_;
  bool touched_;

  void setBtnFunc(const std::string& name, const WidgetsMap& widgets, TouchMenu *tm)
  {
    WidgetsMap::WidgetPtr widget = widgets.get(name);
    btn_widgets_.push_back(widget);
    tm->add(widget, std::tr1::bind(&EarthTexture::btnMonth,
                                   this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));
  }

public:
  explicit EarthTexture(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
    texture_index_(env.earth_texture),
    chg_delay_(),
    delay_(params_["earth_delay"].get<double>()),
    inout_(),
    menu_open_(),
    btn_disp_(),
    col_(1),
    menu_width_(params_["earth_menu_width"].get<double>()),
    ease_mode_(QUART_OUT),
    time_cur_(),
    time_ed_(1.5),
    vec_(),
    vec_st_(-100, 0, 0),
    vec_ed_(0, 0, 1),
    btn_alpha_(),
    btn_alpha_st_(),
    btn_alpha_ed_(),
    draw_(),
    touched_()
  {
    DOUT << "EarthTexture()" << std::endl;

    const WidgetsMap widgets(params_["earth_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
    TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
  
    widget_ = widgets.get("earth");
    tm->add(widget_, std::tr1::bind(&EarthTexture::btnEarth,
                                    this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));

    static const char *tbl[] = {
      "e_auto",
      "e_1", 
      "e_2", 
      "e_3", 
      "e_4", 
      "e_5", 
      "e_6", 
      "e_7", 
      "e_8", 
      "e_9", 
      "e_10", 
      "e_11", 
      "e_12",
      "e_n",
    };
    int num = isAchievementUnlock("nightmode", env) ? elemsof(tbl) : elemsof(tbl) - 1;
    for(int i = 0; i < num; ++i)
    {
      this->setBtnFunc(tbl[i], widgets, tm);
    }
  }
  ~EarthTexture() {
    DOUT << "~EarthTexture()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (delay_ > 0.0)
    {
      draw_ = (delay_ -= delta_time) <= 0.0;
      if (!draw_) return;
    }
    
    if (time_cur_ < time_ed_) time_cur_ += delta_time;

    if (inout_)
    {
      Easing easing;
      easing.ease(vec_, time_cur_, vec_st_, vec_ed_, time_ed_, BACK_OUT);
      easing.ease(btn_alpha_, time_cur_, btn_alpha_st_, btn_alpha_ed_, time_ed_, ease_mode_);
      if (time_cur_ >= time_ed_)
      {
        inout_ = false;
        if (!menu_open_) btn_disp_ = false;
      }
    }
    else
    {
      Easing easing;
      easing.ease(vec_, time_cur_, vec_st_, vec_ed_, time_ed_, ease_mode_);
    }

    if (chg_delay_ > 0.0)
    {
      if ((chg_delay_ -= delta_time) <= 0.0)
      {
        env_.earth_texture = texture_index_;
        env_.task->sendMsgAll(MSG_TITLE_EARTH_TEX_CHANGE);
      }
    }

    if (touched_ && (time_cur_ >= time_ed_))
    {
      active_ = false;
    }
  }
  
  void draw()
  {
    if (!draw_) return;

    {
      const Vec2<float>& pos = widget_->posOrig();
      widget_->setPos(pos.x + vec_.x, pos.y + vec_.y);
      widget_->setCol(col_, col_, col_, vec_.z);
      widget_->draw();
    }

    if (btn_disp_)
    {
      for (std::vector<WidgetsMap::WidgetPtr>::iterator it = btn_widgets_.begin(); it != btn_widgets_.end(); ++it)
      {
        const Vec2<float>& pos = (*it)->posOrig();
        (*it)->setPos(pos.x + vec_.x, pos.y + vec_.y);
        float c = texture_index_ == (*it)->id() ? 1 : 0.5;
        (*it)->setCol(c, c, c, btn_alpha_ * vec_.z);
        (*it)->draw();
      }
    }
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_TITLE_INTRO_SKIP:
      {
        draw_ = true;
        delay_ = 0.0;
        time_cur_ = 100;
      }
      break;

    case MSG_TITLE_EARTH_TEX_OPEN:
    case MSG_TITLE_EARTH_TEX_CLOSE:
      {
        vec_st_ = vec_;
        vec_ed_.set(menu_open_ ? 0 : menu_width_, 0, 1);
        btn_alpha_st_ = btn_alpha_;
        btn_alpha_ed_ = menu_open_ ? 0 : 1;
        time_cur_ = 0;
        time_ed_ = 0.5;
        menu_open_ = !menu_open_;
        if (menu_open_) btn_disp_ = true;
        
        inout_ = true;
      }
      break;
      
    case MSG_TITLE_CREDITS_PUSH:
    case MSG_TITLE_GAMESTART:
    case MSG_TITLE_END:
      if (!touched_)
      {
        ease_mode_ = QUART_IN;
        time_cur_ = 0.0;
        time_ed_ = 0.5;
        vec_st_ = vec_;
        vec_ed_.set(-100, 0, 0);

        touched_ = true;
      }
      break;
    }
  }

  
  void btnEarth(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (!draw_ || touched_ || (chg_delay_ > 0.0)) return;

    switch (type)
    {
    case TouchMenu::CANCEL_IN:
      {
        col_ = 1.0;
        env_.task->sendMsgAll(menu_open_ ? MSG_TITLE_EARTH_TEX_CLOSE : MSG_TITLE_EARTH_TEX_OPEN);
      }
      break;

    case TouchMenu::TOUCH:
    case TouchMenu::MOVE_IN_EDGE:
      {
        col_ = 0.5;
      }
      break;

    case TouchMenu::MOVE_OUT_EDGE:
    case TouchMenu::CANCEL_OUT:
      {
        col_ = 1.0;
      }
      break;
    }
  }

  void btnMonth(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (!draw_ || touched_ || (chg_delay_ > 0.0)) return;

    if (type == TouchMenu::CANCEL_IN)
    {
      texture_index_ = widget.id();
      chg_delay_ = 0.05;
    }
  }
  
};

}
