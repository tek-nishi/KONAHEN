
#pragma once

//
// 中断メニューとか
//

#include <iostream>
#include <string>
#include "co_easing.hpp"
#include "nn_gameenv.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class GameMenu : public TaskProc {
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  bool inout_;                                      // 表示開始＆終了
  bool disp_;
  bool disp_menu_;
  bool disp_eft_;
  float disp_cur_, disp_ed_;
  float ofs_;
  float ofs_st_, ofs_ed_;
  bool exec_fin_;                                   // 演出後終了

  float time_;
  float alpha_;

  WidgetsMap::WidgetPtr widget_;
  WidgetsMap::WidgetPtr ok_widget_;
  WidgetsMap::WidgetPtr ng_widget_;
  WidgetsMap::WidgetPtr pause_widget_;
  
public:
  explicit GameMenu(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["gamemenu"].get<picojson::object>()),
    inout_(true),
    disp_(),
    disp_menu_(),
    disp_eft_(),
    disp_cur_(),
    disp_ed_(1.0),
    ofs_(),
    ofs_st_(100.0),
    ofs_ed_(),
    exec_fin_(),
    time_(),
    alpha_()
  {
    DOUT << "GameMenu()" << std::endl;

    const WidgetsMap widgets(params_["widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
    widget_ = widgets.get("abort");
    ok_widget_ = widgets.get("abort_ok");
    ng_widget_ = widgets.get("abort_ng");
    pause_widget_ = widgets.get("pause");

    TouchMenu *tm = static_cast<TouchMenu *>(env_.touchMenu.get());
    tm->add(widget_, std::tr1::bind(&GameMenu::btnAbort,
                                    this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));
    tm->add(ok_widget_, std::tr1::bind(&GameMenu::btnAbortOk,
                                       this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));
    tm->add(ng_widget_, std::tr1::bind(&GameMenu::btnAbortNg,
                                       this, std::tr1::placeholders::_1, std::tr1::placeholders::_2, std::tr1::placeholders::_3));

    if (env_.demo)
    {
      active_ = false;
      // デモプレイの場合は即時終了
    }
  }
  ~GameMenu()
  {
    DOUT << "~GameMenu()" << std::endl;
  }

  bool active() const { return active_; }

  void step(const float delta_time)
  {
    if (disp_cur_ < disp_ed_) disp_cur_ += delta_time;

    if (inout_)
    {
      Easing easing;
      easing.ease(ofs_, disp_cur_, ofs_st_, ofs_ed_, disp_ed_, QUART_OUT);
      inout_ = (disp_cur_ < disp_ed_);
      if (exec_fin_ && !inout_) active_ = false;
    }
    else
    if (disp_eft_)
    {
      Easing easing;
      easing.ease(ofs_, disp_cur_, ofs_st_, ofs_ed_, disp_ed_, BACK_OUT);
      if (disp_cur_ > disp_ed_)
      {
        disp_eft_ = false;
        ofs_ = ofs_ed_;
        disp_menu_ = disp_;
      }
    }

    if (disp_)
    {
      time_ += delta_time;
      alpha_ = (int)(time_ * 1.5f) & 0x1 ? 1 : 0;
    }

  }

  void draw()
  {
    const Vec2<float>& pos = widget_->posOrig();
    widget_->setPos(pos.x + ofs_, pos.y);
    widget_->draw();

    if (disp_menu_)
    {
      {
        const Vec2<float>& pos = ok_widget_->posOrig();
        ok_widget_->setPos(pos.x + ofs_, pos.y);
        ok_widget_->draw();
      }
      {
        const Vec2<float>& pos = ng_widget_->posOrig();
        ng_widget_->setPos(pos.x + ofs_, pos.y);
        ng_widget_->draw();
      }
    }

    if (disp_)
    {
      pause_widget_->setCol(1, 1, 1, alpha_);
      pause_widget_->draw();
    }
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_END:
      {
//        active_ = false;
        inout_ = true;
        disp_cur_ = 0.0f;
        disp_ed_ = 1.0f;
        ofs_st_ = 0.0f;
        ofs_ed_ = 100.0f;
        exec_fin_ = true;
      }
      break;

    case MSG_GAME_CLEANUP:
      {
        active_ = false;
      }
      break;

    case MSG_GAME_MENU_OFF:
      {
        // inout_ = true;
        // disp_cur_ = 0.0f;
        // disp_ed_ = 1.0f;
        // ofs_st_ = 0.0f;
        // ofs_ed_ = 100.0f;
      }
      break;
    }
  }

  void menuOnOff()
  {
    ofs_st_ = ofs_;
    ofs_ed_ = disp_ ? 0.0 : params_["ofs"].get<double>(); 
    disp_eft_ = true;
    disp_cur_ = 0.0;
    disp_ed_ = 0.5;
    disp_ = !disp_;
    disp_menu_ = true;
    env_.task->sendMsgAll(MSG_GAME_PAUSE);
  }

  void btnAbort(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (inout_) return;

    switch (type)
    {
    case TouchMenu::CANCEL_IN:
      {
        widget.setCol(1, 1, 1, 1);
        this->menuOnOff();
        time_ = 0.0;
      }
      break;

    case TouchMenu::TOUCH:
    case TouchMenu::MOVE_IN_EDGE:
      {
        widget.setCol(0.5, 0.5, 0.5, 1);
      }
      break;

    case TouchMenu::MOVE_OUT_EDGE:
    case TouchMenu::CANCEL_OUT:
      {
        widget.setCol(1, 1, 1, 1);
      }
      break;
    }
  }
  
  void btnAbortOk(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (!disp_ || inout_) return;

    switch (type)
    {
    case TouchMenu::CANCEL_IN:
      {
        widget.setCol(1, 1, 1, 1);

        env_.task->sendMsgAll(MSG_RESETCAMERA_ABORT);
        
        Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
        ResetCamera *cam = static_cast<ResetCamera *>(t.get());
        cam->type(QUAD_INOUT);
        cam->time(3.0);
        if (env_.settings->get<bool>("played"))
        {
          cam->randomDist();
          // cam->randomRot();
        }
        else
        {
          picojson::object& params = env_.params->value().get<picojson::object>();
          picojson::array& array = params["start_angle"].get<picojson::array>();
          Vec2<float> v(Ang2Rad(array[0].get<double>()), Ang2Rad(array[1].get<double>()));
          cam->rotate(v);
        }

        env_.earth->setRotSpeed(env_.earth->getRotSpeedOrig(), 1.0);
        env_.task->sendMsgAll(MSG_GAME_CONTROL_STOP);
          
        env_.cleanup = true;
        // FIXME: メッセージ方式じゃないのが微妙
      }
      break;

    case TouchMenu::TOUCH:
    case TouchMenu::MOVE_IN_EDGE:
      {
        widget.setCol(0.5, 0.5, 0.5, 1);
      }
      break;

    case TouchMenu::MOVE_OUT_EDGE:
    case TouchMenu::CANCEL_OUT:
      {
        widget.setCol(1, 1, 1, 1);
      }
      break;
    }
  }

  void btnAbortNg(const int type, const Vec2<float>& pos, Widget& widget)
  {
    if (!disp_ || inout_) return;


    switch (type)
    {
    case TouchMenu::CANCEL_IN:
      {
        widget.setCol(1, 1, 1, 1);
        this->menuOnOff();
      }
      break;

    case TouchMenu::TOUCH:
    case TouchMenu::MOVE_IN_EDGE:
      {
        widget.setCol(0.5, 0.5, 0.5, 1);
      }
      break;

    case TouchMenu::MOVE_OUT_EDGE:
    case TouchMenu::CANCEL_OUT:
      {
        widget.setCol(1, 1, 1, 1);
      }
      break;
    }
  }
};
  
}
