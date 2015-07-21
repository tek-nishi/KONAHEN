
#pragma once

//
// スタッフ表示
//

#include <iostream>
#include <string>
#include <vector>
#include "co_texmng.hpp"
#include "co_task.hpp"
#include "co_fntmng.hpp"
#include "nn_gameenv.hpp"
#include "nn_agree.hpp"


namespace ngs {

#ifdef VER_LITE
const char staff_key[] = "staff_lite";
#else
const char staff_key[] = "staff";
#endif

class Staff : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;
  const picojson::array& array_;
  const TexMng::tex_ptr texture_;

  FntMng::FontPtr font_;
  int height_;
  float y_ofs_;

  struct StaffDisp {
    const std::string *text;
    Vec2<float> pos;
  };
  std::vector<StaffDisp> disp_;
  Vec2<float> size_;

  int staff_ease_;
  float time_cur_, time_ed_;
  float alpha_;
  float alpha_st_, alpha_ed_;
  float scale_;
  float scale_st_, scale_ed_;

  float agree_delay_;
  bool touched_;
  float fin_delay_;
  float fin_msg_delay_;

public:
  explicit Staff(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()[staff_key].get<picojson::object>()),
    array_(params_["text"].get<picojson::array>()),
    texture_(env.texMng->read(*env.path + "devdata/round.png")),
    font_(ReadFont("staff", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
    height_(font_->height()),
    y_ofs_(params_["y_ofs"].get<double>()),
    staff_ease_(CUBIC_OUT),
    time_cur_(),
    time_ed_(2),
    alpha_st_(),
    alpha_ed_(1),
    scale_(),
    scale_st_(0.25),
    scale_ed_(1.0),
    agree_delay_(2),
    touched_(),
    fin_delay_(),
    fin_msg_delay_()
  {
    DOUT << "Staff()" << std::endl;

    // 事前に表示領域を計算しておく
    float max_x = 0;
    float max_y = -10;
    float y = (array_.size() * (height_ + 10)) / -2.0;
    for (picojson::array::const_iterator it = array_.begin(); it != array_.end(); ++it)
    {
      const std::string& text = it->get<std::string>();

      const Vec2<float>& size = font_->size(text);
      float x = size.x / -2.0;
      if (max_x < size.x) max_x = size.x;

      StaffDisp disp = { &text, Vec2<float>(x, y) };
      disp_.push_back(disp);
      
      y += height_ + 10;
      max_y += height_ + 10;
    }
    size_.set(max_x + 40, max_y + 40);
    
  }

  ~Staff()
  {
    DOUT << "~Staff()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (time_cur_ < time_ed_) time_cur_ += delta_time;
    Easing easing;
    easing.ease(alpha_, time_cur_, alpha_st_, alpha_ed_, time_ed_, staff_ease_);
    easing.ease(scale_, time_cur_, scale_st_, scale_ed_, time_ed_, staff_ease_);

    if ((agree_delay_ > 0) && ((agree_delay_ -= delta_time) <= 0))
    {
      env_.task->add<Agree>(TASK_PRIO_2D, env_);
    }

    if (touched_)
    {
      touched_ = false;

      staff_ease_ = QUAD_IN;
      time_cur_ = 0;
      time_ed_ = 1;
      alpha_st_ = alpha_;
      alpha_ed_ = 0;
      scale_st_ = 1.0;
      scale_ed_ = 0.25;
      fin_delay_ = time_ed_;
      fin_msg_delay_ = 0.25;
    }

    if (fin_msg_delay_ > 0 && (fin_msg_delay_ -= delta_time) <= 0)
    {
      env_.task->sendMsgAll(MSG_TITLE_CREDITS_FIN);
    }

    if (fin_delay_ > 0 && (fin_delay_ -= delta_time) <= 0)
    {
      active_ = false;
    }
  }
  
  void draw()
  {
    glPushMatrix();

#if (TARGET_OS_IPHONE)
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#endif

    glScalef(scale_, scale_, 1.0);
    glTranslatef(0, y_ofs_, 0);

    font_->col(1, 1, 1, alpha_);

    GrpRoundBox obj;
    obj.pos(0, -height_ - 3);
    obj.size(size_);
    obj.center();
    obj.texture(texture_);
    obj.col(0, 0, 0, 0.6 * alpha_);
    obj.draw();
    
    for(std::vector<StaffDisp>::const_iterator it = disp_.begin(); it != disp_.end(); ++it)
    {
      const Vec2<float>& pos = it->pos;
      const std::string& text = *(it->text);
      font_->pos(pos);
      font_->draw(text);
    }

#if (TARGET_OS_IPHONE)
    glPopClientAttrib();
#endif

    glPopMatrix();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_AGREE:
      {
        touched_ = true;
      }
      break;
    }
  }
};

}
