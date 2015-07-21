
#pragma once

//
// 「了解」入力
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "co_easearray.hpp"
#include "nn_gameenv.hpp"
#include "nn_widget.hpp"


namespace ngs {

class Agree : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  float alpha_;
  float alpha_st_, alpha_ed_;
  float time_cur_;
  float time_ed_;

  struct BtnScale {
    bool ease;
    float alpha;
    float alpha_st, alpha_ed;
    float time, time_ed;
    int mode;
    BtnScale() :
      ease(),
      alpha(1),
      alpha_st(),
      alpha_ed(),
      time(),
      time_ed(),
      mode()
    {}
  };
  BtnScale btn_scale_;

  const TexMng::tex_ptr texture_;
  WidgetsMap::WidgetPtr widget_;
  Vec2<float> pos_;

  bool touched_;
  
public:
  explicit Agree(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["agree"].get<picojson::object>()),
    alpha_(),
    alpha_st_(),
    alpha_ed_(1),
    time_cur_(),
    time_ed_(0.5),
    texture_(env_.texMng->read(*env.path + "devdata/game2.png")),
    widget_(new Widget(params_["widget"].get<picojson::object>(), texture_, env_.size, env_.y_bottom)),
    pos_(widget_->posOrig()),
    touched_()
  {
    DOUT << "Agree()" << std::endl;

#ifdef VER_LITE
    float y = params_["lite_ofs"].get<double>();
    pos_.y += y;
    widget_->setPos(pos_);
#endif
    
    TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
    tm->add(widget_, std::tr1::bind(&Agree::btnAgree,
                                    this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));
  }
  ~Agree() {
    DOUT << "~Agree()" << std::endl;
  }

  bool active() const { return active_; }

#if 0
  void pos(const Vec2<float>& pos)
  {
    pos_ = pos;
  }
#endif
  
  void step(const float delta_time)
  {
    if (time_cur_ < time_ed_) time_cur_ += delta_time;
    Easing easing;
    easing.ease(alpha_, time_cur_, alpha_st_, alpha_ed_, time_ed_);
    if (btn_scale_.ease)
    {
      btn_scale_.time += delta_time;
      btn_scale_.ease = (btn_scale_.time <= btn_scale_.time_ed);
      easing.ease(btn_scale_.alpha, btn_scale_.time, btn_scale_.alpha_st, btn_scale_.alpha_ed, btn_scale_.time_ed, btn_scale_.mode);
    }
    
    if (touched_ && (time_cur_ >= time_ed_))
    {
      active_ = false;
    }
  }
  
  void draw()
  {
    widget_->setPos(pos_);
    widget_->setScale(btn_scale_.alpha, btn_scale_.alpha);
    widget_->setCol(1, 1, 1, alpha_);
    widget_->draw();
  }

  void msg(const int msg) {}

  void btnAgree(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (touched_) return;

    switch (type)
    {
    case TouchMenu::TOUCH:
    case TouchMenu::MOVE_IN_EDGE:
      {
        btn_scale_.ease = true;
        btn_scale_.alpha_st = btn_scale_.alpha;
        btn_scale_.alpha_ed = params_["btneft_scale"].get<double>();
        btn_scale_.time = 0;
        btn_scale_.time_ed = params_["btneft_time"].get<double>();
        btn_scale_.mode = BACK_OUT;
      }
      break;

    case TouchMenu::CANCEL_IN:
      {
        env_.sound->play("touch_out");
      
        env_.task->sendMsgAll(MSG_GAME_AGREE);
        alpha_st_ = alpha_;
        alpha_ed_ = 0;
        time_cur_ = 0;
        touched_ = true;
      }
      break;

    case TouchMenu::MOVE_OUT_EDGE:
    case TouchMenu::CANCEL_OUT:
      {
        btn_scale_.ease = true;
        btn_scale_.alpha_st = btn_scale_.alpha;
        btn_scale_.alpha_ed = 1;
        btn_scale_.time = 0;
        btn_scale_.time_ed = params_["btneft_time"].get<double>();
        btn_scale_.mode = BACK_OUT;
      }
      break;
    }
  }
  
};

}

