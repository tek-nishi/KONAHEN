
#pragma once

//
// タイトルロゴ
//

#include <iostream>
#include <string>
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {


class TitleLogo : public TaskProc, TouchCallBack {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  struct Logo {
    WidgetsMap::WidgetPtr widget;
    EaseArray<Vec3<float> > easing;
    Vec3<float> vec;
  };
  std::vector<Logo> logos_;
  
  WidgetsMap::WidgetPtr konahen_kana_widget_;
  WidgetsMap::WidgetPtr ngs_widget_;
  WidgetsMap::WidgetPtr lite_widget_;
  
  bool draw_;
  float delay_;

  EaseArray<float> ease2Array_;
  float time_;
  float alpha_;

  float ofs_;

  bool touched_;
  bool firstexec_;
  bool finish_;

  void logoWidgetsSetup(const std::string& widget, const picojson::object& params, const WidgetsMap& widgets)
  {
    const picojson::array& array = params.find(widget)->second.get<picojson::array>();
    for(picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
    {
      Logo logo;
      logo.widget = widgets.get(it->get<std::string>());
      logos_.push_back(logo);
    }
  }

  void logoEasingSetup(const int type, const std::string& easing, const picojson::object& params)
  {
    const picojson::array& array = params.find(easing)->second.get<picojson::array>();
    for(u_int i = 0; i < logos_.size(); ++i)
    {
      const picojson::array& a = params.find(array[i].get<std::string>())->second.get<picojson::array>();
      EasingArayVec3(type, a, logos_[i].easing);
    }
  }
  
public:
  explicit TitleLogo(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
    draw_(),
    delay_(params_["logo_delay"].get<double>()),
    time_(),
    alpha_(),
    ofs_(),
    touched_(),
    firstexec_(env_.settings->get<bool>("firstexec")),
    finish_()
  {
    DOUT << "TitleLogo()" << std::endl;

    const WidgetsMap widgets(params_["logo_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);

    logoWidgetsSetup("logo_widgets_list", params_, widgets);
    logoEasingSetup(QUART_OUT, "logo_easing", params_);

    konahen_kana_widget_ = widgets.get("konahen_kana");
    ngs_widget_ = widgets.get("ngs");
    lite_widget_ = widgets.get("lite");

    EasingArayFloat(CUBIC_OUT, params_["logo2_easing"].get<picojson::array>(), ease2Array_);

    const bool advanced = isAchievementUnlock("advanced", env_);
    const bool survival = isAchievementUnlock("survival", env_);
    const bool review = isAchievementUnlock("review", env_, true);
#ifdef VER_LITE
    const bool getfull = isAchievementUnlock("getfull", env_, true);
#endif

    int num = 1;
    if (advanced) ++num;
    if (survival) ++num;
    if (review) ++num;
#ifdef VER_LITE
    if (getfull) ++num;
#endif
    switch (num)
    {
    case 3:
      ofs_ = -40.0f / 2.0f;
#ifdef VER_LITE
      ofs_ -= 5.0f;
#endif
      break;

    case 4:
      ofs_ = -40.0f;
#ifdef VER_LITE
      ofs_ -= 5.0f;
#endif
      break;
      
    }
    // メニューのボタンの数を調べて位置を調整

    env_.touch->resistCallBack(this);
  }
  
  ~TitleLogo() {
    DOUT << "~TitleLogo()" << std::endl;
    env_.touch->removeCallBack(this);
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (delay_ > 0.0)
    {
      draw_ = (delay_ -= delta_time) <= 0.0;
      if (!draw_) return;
    }

    time_ += delta_time;

    bool logo_exec = false;
    for (u_int i = 0; i < logos_.size(); ++i)
    {
      bool exec = logos_[i].easing.ease(logos_[i].vec, time_);
      if (exec) logo_exec = true;
      // ロゴのイージング
    }
    
    bool logo2_exec = ease2Array_.ease(alpha_, time_);
    if ((logo_exec || logo2_exec) && !finish_ && touched_)
    {
      env_.task->sendMsgAll(MSG_TITLE_INTRO_SKIP);
    }

    if (finish_ && !logo_exec && !logo2_exec) active_ = false;
  }
  
  void draw()
  {
    if (!draw_) return;

    glPushMatrix();
    glTranslatef(0.0f, ofs_, 0);

    for (u_int i = 0; i < logos_.size(); ++i)
    {
      const Vec2<float>& pos = logos_[i].widget->posOrig();
      logos_[i].widget->setPos(pos.x + logos_[i].vec.x, pos.y);
      logos_[i].widget->setScale(logos_[i].vec.y, logos_[i].vec.y);
      logos_[i].widget->setCol(1.0, 1.0, 1.0, logos_[i].vec.z);
      logos_[i].widget->draw();
    }

    konahen_kana_widget_->setCol(1.0, 1.0, 1.0, alpha_);
    konahen_kana_widget_->draw();
    ngs_widget_->setCol(1.0, 1.0, 1.0, alpha_);
    ngs_widget_->draw();

#ifdef VER_LITE
    {
      lite_widget_->setCol(1.0, 1.0, 1.0, alpha_);
      lite_widget_->draw();
    }
#endif
    glPopMatrix();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_TITLE_INTRO_SKIP:
      {
        delay_ = 0;
        draw_ = true;
        time_ = 100;
        // イージングが終了する充分な時間
      }
      break;

    case MSG_TITLE_CREDITS_PUSH:
    case MSG_TITLE_END:
      if (!finish_)
      {
        time_ = 0;
        finish_ = true;

        logoEasingSetup(QUART_IN, "logo_end_easing", params_);
        EasingArayFloat(CUBIC_OUT, params_["logo2_end_easing"].get<picojson::array>(), ease2Array_);
      }
      break;
      
    case MSG_TITLE_GAMESTART:
      if (!finish_)
      {
        time_ = 0;
        finish_ = true;

        logoEasingSetup(QUART_IN, "logo_start_easing", params_);
        EasingArayFloat(CUBIC_OUT, params_["logo2_start_easing"].get<picojson::array>(), ease2Array_);
      }
      break;
    }
  }

  void touchStart(const Touch& touch, const std::vector<TouchInfo>& info) {}
  void touchMove(const Touch& touch, const std::vector<TouchInfo>& info) {}
  void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info)
  {
    if (!draw_ || firstexec_ || touched_) return;
    touched_ = true;
  }
};

}
