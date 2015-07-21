
#pragma once

//
// 復習モード・スクロールバー
//


namespace ngs {

class ReviewBar : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;
  const std::vector<int>& place_levels_;

  const TexMng::tex_ptr texture_;
  const float interval_;

  WidgetsMap::WidgetPtr bar_;
  WidgetsMap::WidgetPtr btn_big_;
  WidgetsMap::WidgetPtr center_up_;
  WidgetsMap::WidgetPtr center_down_;
  WidgetsMap::WidgetPtr left_;
  WidgetsMap::WidgetPtr right_;
  WidgetsMap::WidgetPtr left_arrow_;
  WidgetsMap::WidgetPtr right_arrow_;

  const float bar_margin_;
  const float bar_min_;

  EaseArray<Vec2<float> > ease_bar_;
  bool bar_eft_;
  Vec2<float> bar_ofs_;
  float ease_time_;

  float arrow_alpha_;
  EaseArray<float> ease_arrow_;
  float arrow_alpha_time_;
  bool ease_arrow_alpha_;
  bool fade_arrow_alpha_;

  float last_pos_x_;
  float drag_ofs_;
  float drag_speed_;
  float drag_speed_old_;
  const float drag_decay_;
  const float drag_stick_;
  bool touched_;
  
  const float appeal_speed_;
  const float appeal_width_;
  float appeal_;

  bool fade_;
  float fade_time_;
  float fade_alpha_;
  float fade_alpha_st_;
  float fade_alpha_ed_;
  float fade_alpha_time_;
  
  bool pause_;

  void drawBar(const Vec2<float>& pos, const Vec2<float>& size, const float bar_ofs, const GrpCol<float>& col, const int index)
  {
    float scale = ((bar_ofs < (interval_ / 2.0f)) && (bar_ofs >= (-interval_ / 2.0f))) ? 1.2f :
      (place_levels_[index] & 1) ? 0.6f : 0.4f;

    float t = (bar_ofs >= 0.0f) ? bar_ofs / (size.x / 2.0f - 5.0f) : bar_ofs / (size.x / -2.0f + 5.0f);
    float a;
    Easing easing;
    easing.ease(a, t, 1.0f, 0.0f, 1.0f, QUART_IN);
    btn_big_->setCol(col.r, col.g, col.b, col.a * a);
    btn_big_->setPos(pos.x + bar_ofs, pos.y);
    btn_big_->setScale(scale, scale);
    btn_big_->draw();
  }
  
public:
  ReviewBar(GameEnv& env, picojson::object& params, const std::vector<int>& place_levels) :
    env_(env),
    active_(true),
    params_(params),
    place_levels_(place_levels),
    texture_(env.texMng->read(*env.path + "devdata/round.png")),
    interval_(params["interval"].get<double>()),
    bar_margin_(params["bar_margin"].get<double>()),
    bar_min_(params["bar_min"].get<double>()),
    bar_eft_(true),
    ease_time_(),
    arrow_alpha_(),
    arrow_alpha_time_(),
    ease_arrow_alpha_(true),
    fade_arrow_alpha_(),
    last_pos_x_(),
    drag_ofs_(),
    drag_speed_(),
    drag_speed_old_(),
    drag_decay_(params["drag_decay"].get<double>()),
    drag_stick_(params["drag_stick"].get<double>()),
    touched_(),
    appeal_speed_(params["appeal_speed"].get<double>()),
    appeal_width_(params["appeal_width"].get<double>()),
    appeal_(),
    fade_(),
    fade_alpha_(1),
    pause_()
  {
    DOUT << "ReviewBar()" << std::endl;

    const WidgetsMap widgets(params_["widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
    bar_ = widgets.get("bar");
    btn_big_ = widgets.get("btn_big");
    center_up_ = widgets.get("center_up");
    center_down_ = widgets.get("center_down");
    left_ = widgets.get("left");
    right_ = widgets.get("right");
    left_arrow_ = widgets.get("left_arrow");
    right_arrow_ = widgets.get("right_arrow");

    EasingArayVec2(QUART_OUT, params_["easing_bar"].get<picojson::array>(), ease_bar_);
    EasingArayFloat(QUART_OUT, params_["easing_allow"].get<picojson::array>(), ease_arrow_);

    {
      using namespace std::tr1;
      TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
      tm->add(bar_, bind(&ReviewBar::btnProc, this,
                         placeholders::_1, placeholders::_2, placeholders::_3));
    }
  }
  
  ~ReviewBar()
  {
    DOUT << "~ReviewBar()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (fade_)
    {
      fade_time_ += delta_time;

      Easing easing;
      easing.ease(fade_alpha_, fade_time_, fade_alpha_st_, fade_alpha_ed_, fade_alpha_time_);
      if (fade_time_ >= fade_alpha_time_) fade_ = false;
      // メニュー表示時はフェードで消す処理
    }

    {
      const Vec2<float>& size = bar_->size();
      float bar_size = env_.size->x - bar_margin_;
      bar_->setSize((bar_size > bar_min_) ? bar_size : bar_min_, size.y);
      // 縦・横でバーの長さを変える
    }
    
    if (pause_) return;

    ease_time_ += delta_time;
    if (bar_eft_)
    {
      bar_eft_ = ease_bar_.ease(bar_ofs_, ease_time_);
    }
    
    if (ease_arrow_alpha_)
    {
      arrow_alpha_time_ += delta_time;
      ease_arrow_alpha_ = ease_arrow_.ease(arrow_alpha_, arrow_alpha_time_);
      // 操作指示の矢印を表示
    }
    else
    if (fade_arrow_alpha_)
    {
      arrow_alpha_ -= 2.0f * delta_time;
      if (arrow_alpha_ < 0.0f)
      {
        arrow_alpha_ = 0.0f;
        fade_arrow_alpha_ = false;
      }
      // 操作されたら操作指示の矢印は消す
    }

    if (!touched_)
    {
      float t = delta_time / (1.0f / 60.0f);
      drag_speed_ *= pow(drag_decay_, t);
      // 近似的に減衰運動を経過時間で導く手法
      drag_ofs_ += drag_speed_;

      float val = fmod(drag_ofs_, interval_);
      float v = val;
      if (val > (interval_ / 2.0f)) v = -(interval_ - val);
      else if (val < (interval_ / -2.0f)) v = -(-interval_ - val);
      v = v * pow(drag_stick_, t);
      drag_ofs_ -= v;
      // 目盛りに吸着する処理
    }
    else
    {
      drag_speed_old_ = drag_speed_;
      drag_speed_ = 0;
    }

    float ofs = (drag_ofs_ >= 0.0f) ? interval_ / 2.0f : -interval_ / 2.0f;
    // 目盛の半分の位置で切り替えるための加算値

    int idx = (drag_ofs_ + ofs) / interval_;
    if (idx < 0)
    {
      idx = -idx % env_.question_total;
      // 1, 2, 3, 4, ...
    }
    else
    {
      idx = (env_.question_total - (idx % env_.question_total)) % env_.question_total;
      // 0, 5, 4, 3, 2, ...
    }
    
    if (env_.question != idx)
    {
      env_.question = idx;
      env_.task->sendMsgAll(MSG_REVIEW_UPDATE);
    }

    appeal_ += appeal_speed_ * delta_time;
  }

  void draw()
  {
    glPushMatrix();
    glTranslatef(0, bar_ofs_.x, 0);

    const GrpCol<float> col(1, 1, 1, bar_ofs_.y * fade_alpha_);
    {
      const Vec2<float>& pos = bar_->dispPos();
      const Vec2<float>& size = bar_->size();
      GrpRoundBox obj;
      obj.pos(pos);
      obj.size(size);
      obj.center(0, 0);
      obj.texture(texture_);
      obj.col(0, 0, 0, 0.5f * col.a);
      obj.draw();
      // サイズを可変にしたかったので自前で描画
    }

    center_down_->setCol(col);
    center_down_->draw();

    const float app_ofs = fabs(sin(appeal_) * appeal_width_);
#if 0
    {
      const Vec2<float>& pos = left_->posOrig();
      left_->setPos(pos.x - app_ofs, pos.y);
      left_->setCol(col);
      left_->draw();
    }
    {
      const Vec2<float>& pos = right_->posOrig();
      right_->setPos(pos.x + app_ofs, pos.y);
      right_->setCol(col);
      right_->draw();
    }
    // 左右の矢印をピョコピョコ動かしつつ表示
#endif
    
    const Vec2<float>& pos = btn_big_->posOrig();
    const Vec2<float>& size = bar_->size();
    float ofs = fmod(drag_ofs_, interval_);

    int start_ofs = env_.question;
    if (ofs >= (interval_ / 2.0f))
    {
      start_ofs = (start_ofs + 1) % env_.question_total;
    }
    else
    if (ofs < (-interval_ / 2.0f))
    {
      start_ofs -= 1;
      if (start_ofs < 0) start_ofs += env_.question_total;
    }
    int start_ofs_left = (start_ofs + 1) % env_.question_total;
    // 描画開始位置の問題番号の決定

    for(float bar_ofs = ofs; bar_ofs > (size.x / -2.0f + 5.0f); bar_ofs -= interval_)
    {
      GrpCol<float> red(1, 0, 0, col.a);
      drawBar(pos, size, bar_ofs, start_ofs == 0 ? red : col, start_ofs);
      // マイナス方向の描画

      --start_ofs;
      if (start_ofs < 0) start_ofs += env_.question_total;
    }

    start_ofs = start_ofs_left;
    for(float bar_ofs = ofs + interval_; bar_ofs < (size.x / 2.0f - 5.0f); bar_ofs += interval_)
    {
      GrpCol<float> red(1, 0, 0, col.a);

      drawBar(pos, size, bar_ofs, start_ofs == 0 ? red : col, start_ofs);
      // プラス方向の描画

      start_ofs = (start_ofs + 1) % env_.question_total;
    }

    if (arrow_alpha_ > 0.0f)
    {
      {
        const Vec2<float>& pos = left_arrow_->posOrig();
        left_arrow_->setPos(pos.x - app_ofs, pos.y);
        left_arrow_->setCol(col.r, col.g, col.b, col.a * arrow_alpha_);
        left_arrow_->draw();
      }
      {
        const Vec2<float>& pos = right_arrow_->posOrig();
        right_arrow_->setPos(pos.x + app_ofs, pos.y);
        right_arrow_->setCol(col.r, col.g, col.b, col.a * arrow_alpha_);
        right_arrow_->draw();
      }
      // 左右の矢印をピョコピョコ動かしつつ表示
    }
    
    glPopMatrix();
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

    case MSG_GAME_PAUSE:
      {
        pause_ = !pause_;

        fade_ = true;
        fade_time_ = 0.0;
        fade_alpha_st_ = fade_alpha_;
        fade_alpha_ed_ = pause_ ? 0.0 : 1.0;
        fade_alpha_time_ = pause_ ? 0.2 : 0.4;
      }
      break;
    }
  }

  
  void btnProc(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (pause_) return;

    Vec2<float> wpos = widget.posOrig();
    switch (type)
    {
    case TouchMenu::TOUCH:
      {
        last_pos_x_ = pos.x;
        touched_ = true;
        drag_speed_ = 0.0f;
        // env_.task->sendMsgAll(MSG_REVIEW_CONTROL_STOP);
      }
      break;

    case TouchMenu::CANCEL_IN:
    case TouchMenu::CANCEL_OUT:
      {
        touched_ = false;
        drag_speed_ = drag_speed_old_ * 0.9f;
        drag_speed_old_ = 0.0f;
        // env_.task->sendMsgAll(MSG_REVIEW_CONTROL_START);
      }
      break;

    default:
      {
        // FIXME:マルチタッチの処理がよくないので、
        //       ウィジットとの距離を判定している
        Vec2<float> wpos = widget.dispPos();
        if (fabs(pos.y - wpos.y) < 100.0f)
        {
          float ofs = pos.x - last_pos_x_;
          drag_ofs_ += ofs;
          drag_speed_ = ofs;
          last_pos_x_ = pos.x;

          if (ease_arrow_alpha_ || (arrow_alpha_ > 0.0f))
          {
            ease_arrow_alpha_ = false;
            fade_arrow_alpha_ = true;
            // 操作されたら操作指示の矢印は消す
          }
        }
      }
      break;
    }
    widget.setPos(wpos);
  }
};

}
  
