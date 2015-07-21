
#pragma once

//
// タイトル
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"
#include "nn_gamemain.hpp"
#include "nn_title_intro.hpp"
#include "nn_title_logo.hpp"
#include "nn_title_copy.hpp"
#include "nn_title_start.hpp"
#include "nn_title_credits.hpp"
#include "nn_sound_vol.hpp"
#include "nn_sound_onoff.hpp"
#include "nn_ranking.hpp"
#include "nn_staff.hpp"
#include "nn_achievement.hpp"
#include "nn_correct_rate.hpp"
#include "nn_earth_texture.hpp"
#include "nn_demoplay.hpp"
#include "nn_operate.hpp"
#include "nn_replay.hpp"
#include "nn_review.hpp"
#include "iad.h"
#include "Twitter.h"


namespace ngs {

#if defined (_DEBUG) && defined (JSON_CONV)
void makeJsonToDz()
{
  // jsonファイルを圧縮する
  {
    std::ifstream fstr("devdata/params.json", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/params.dz", input);
  }

  {
    std::ifstream fstr("devdata/places.json", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/places.dz", input);
  }

  {
    std::ifstream fstr("devdata/results.json", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/results.dz", input);
  }
  {
    std::ifstream fstr("devdata/settings.json", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/settings.dz", input);
  }
  {
    std::ifstream fstr("devdata/replay1.rep", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/replay1.dz", input);
  }
  {
    std::ifstream fstr("devdata/replay2.rep", std::ios::binary);
    std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
    zlibWrite("devdata/replay2.dz", input);
  }
  DOUT << "Parameters ziped." << std::endl;
}

// ローカライズ用に場所の日本語を書き出す
void placeTextWrite(GameEnv& env)
{
  std::ofstream fstr("place.txt");
  if (fstr.is_open())
  {
    std::map<std::string, int> list;
    picojson::object& places = env.places->value().get<picojson::object>();
    for (picojson::object::iterator it = places.begin(); it != places.end(); ++it)
    {
      picojson::object& place = it->second.get<picojson::object>();
      std::string& name = place["name"].get<std::string>();
      list[name] = 1;

      if (place["answer"].is<std::string>())
      {
        std::string& answer = place["answer"].get<std::string>();
        list[answer] = 1;
      }
    }

    for (std::map<std::string, int>::iterator it = list.begin(); it != list.end(); ++it)
    {
      fstr << it->first << "\t" << it->first << std::endl;
    }
  }
}

#endif


class Title : public TaskProc
{
  GameEnv& env_;
  Replay& replay_;
  bool active_;

  picojson::object& params_;

  enum {
    MAIN,
    DEMO,
    RANKING,
    CREDITS,
    ACHIEVEMENT
  };
  int mode_;
  std::vector<int> ranking_;
  int ranking_idx_;

  float start_delay_;
  float disp_time_;
  float delay_exec_;

public:
  Title(GameEnv& env, Replay& replay) :
    env_(env),
    replay_(replay),
    active_(true),
    params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
    mode_(ACHIEVEMENT),
    ranking_idx_(),
    start_delay_(),
    disp_time_(params_["disp_time"].get<double>()),
    delay_exec_()
  {
    DOUT << "Title()" << std::endl;

    ranking_.push_back(GAME_MODE_NORMAL);
    if (isAchievementUnlock("advanced", env_)) ranking_.push_back(GAME_MODE_ADVANCED);
    if (isAchievementUnlock("survival", env_)) ranking_.push_back(GAME_MODE_SURVIVAL);

    env_.task->sendMsgAll(MSG_GAME_CONTROL_STOP);
    env_.task->add<Achievement>(TASK_PRIO_2D, env_);
    // TIPS:ランキング画面で終了しても、次回起動時に実績解除画面を表示する
  }
  ~Title()
  {
    DOUT << "~Title()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {

#if defined (_DEBUG) && defined (JSON_CONV)
    std::size_t key_inp = env_.keyinp->get();
    if (key_inp == 'j')
    {
      makeJsonToDz();
    }
    else
    if (key_inp == 'p')
    {
      placeTextWrite(env_);
    }
#endif

    switch (mode_)
    {
    case MAIN:
      {
        if ((disp_time_ > 0.0) && (disp_time_ -= delta_time) <= 0.0)
        {
          env_.task->sendMsgAll(MSG_TITLE_END);
          env_.task->sendMsgAll(MSG_CONTROL_LOCK);
          mode_ = DEMO;
          delay_exec_ = params_["delay_exec"].get<double>();
          HideGameCenerBtn();
          HideTweetBtn();
        }
        else
        if ((start_delay_ > 0.0) && (start_delay_ -= delta_time) <= 0.0)
        {
          env_.task->sendMsgAll(MSG_TITLE_END);
          env_.demo = false;
          switch (env_.game_mode)
          {
          case GAME_MODE_REVIEW:
            env_.task->add<Review>(TASK_PRIO_SYS, env_);
            active_ = false;
            break;
            // 復習モードを起動してタイトルは終了

#ifdef VER_LITE
          case GAME_MODE_GETFULL:
            getFullVersion();
            env_.task->sendMsgAll(MSG_TITLE_INTRO_FIN);
            disp_time_ = params_["disp_time"].get<double>();
            break;
            // タイトルはそのままで購入ページを起動
#endif

          default:
            env_.task->add<GameMain>(TASK_PRIO_SYS, env_, replay_);
            active_ = false;
            break;
            // ゲーム本編を起動してタイトルは終了
          }
        }
      }
      break;

    case DEMO:
      {
        if (delay_exec_ > 0)
        {
          delay_exec_ -= delta_time;
          if (delay_exec_ <= 0)
          {
            env_.task->add<DemoPlay>(TASK_PRIO_2D, env_, replay_);
            env_.task->add<Operate>(TASK_PRIO_2D, env_);
          }
        }
      }
      break;

    case RANKING:
      {
        if (delay_exec_ > 0)
        {
          delay_exec_ -= delta_time;
          if (delay_exec_ <= 0)
          {
            bool record = false;
            env_.task->add<Ranking>(TASK_PRIO_2D, env_, ranking_[ranking_idx_], record);
            ranking_idx_ = (ranking_idx_ + 1) % ranking_.size();

            Task::ProcPtr t = env_.task->add<FadeLight>(TASK_PRIO_3D, env_);
            FadeLight *fl = static_cast<FadeLight *>(t.get());
            fl->type(QUAD_OUT);
            fl->scale(0.5);
            fl->time(2.0);

//            DispGameCenerBtn();
          }
        }
      }
      break;

    default:
      break;
    }
  }
    
  void draw()
  {
#if 0
    {
      GrpLine obj;
      obj.points(0, env_.size->y / -2.0, 0, env_.size->y / 2.0);
      obj.col(0.0, 1.0, 0.0);
      obj.draw();
    }
#endif
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_ACHIEVEMENT_END:
      {
        mode_ = MAIN;
        env_.sound->play("opening", 1.0, 0.5);
        env_.task->add<TitleIntro>(TASK_PRIO_2D, env_);
      }
      break;
      
    case MSG_TITLE_INTRO_FIN:
      {
#ifdef _DEBUG
        env_.task->dispTaskList();
#endif
        env_.task->add<TitleLogo>(TASK_PRIO_2D, env_);
        env_.task->add<TitleCopy>(TASK_PRIO_2D, env_);
        env_.task->add<TitleStart>(TASK_PRIO_2D, env_);
        env_.task->add<TitleCredits>(TASK_PRIO_2D, env_);
        // env_.task->add<SoundVol>(TASK_PRIO_2D, env_);

        u_int type = Audio::BGM;
        env_.task->add<SoundOnOff>(TASK_PRIO_2D, env_, type);
        type = Audio::SE;
        env_.task->add<SoundOnOff>(TASK_PRIO_2D, env_, type);

        if (env_.settings->get<bool>("played"))
        {
          bool review = false;
          env_.task->add<CorrectRate>(TASK_PRIO_2D, env_, review);
        }

        if (isAchievementUnlock("earth_texture", env_))
        {
          env_.task->add<EarthTexture>(TASK_PRIO_2D, env_);
        }
        env_.task->sendMsgAll(MSG_GAME_CONTROL_START);
        DispGameCenerBtn();
        DispTweetBtn();
      }
      break;

    case MSG_TITLE_CREDITS:
      {
        env_.task->sendMsgAll(MSG_TITLE_END);
        env_.task->add<Staff>(TASK_PRIO_2D, env_);
      }
      break;

    case MSG_TITLE_CREDITS_PUSH:
      {
        mode_ = CREDITS;
      }
      break;

    case MSG_DEMOPLAY_END:
      {
        env_.task->sendMsgAll(MSG_CONTROL_UNLOCK);
        mode_ = RANKING;
        delay_exec_ = params_["delay_exec"].get<double>();
        env_.earth->setRotSpeed(env_.earth->getRotSpeedOrig(), 1.0);

        Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
        ResetCamera *cam = static_cast<ResetCamera *>(t.get());
        cam->type(QUAD_INOUT);
        cam->time(2.0);
        cam->randomDist();
      }
      break;

    case MSG_RANKING_END:
    case MSG_TITLE_CREDITS_FIN:
      {
        env_.task->sendMsgAll(MSG_TITLE_INTRO_FIN);
        // TIPS: イントロを飛ばしていきなりロゴ表示
        
        mode_ = MAIN;
        disp_time_ = params_["disp_time"].get<double>();
      }
      break;

    case MSG_TITLE_SOUND_VOL_OPEN:
    case MSG_TITLE_EARTH_TEX_OPEN:
      {
        disp_time_ = 0;
        // メニュー表示中はランキングへ移行しない
      }
      break;

    case MSG_TITLE_SOUND_VOL_CLOSE:
    case MSG_TITLE_EARTH_TEX_CLOSE:
      {
        disp_time_ = params_["disp_time"].get<double>();
      }
      break;

    case MSG_TITLE_GAMESTART:
      {
        start_delay_ = params_["game_start_delay"].get<double>();
        disp_time_ = 0.0;

        if (env_.game_mode == GAME_MODE_NORMAL)
        {
          // ノーマルモードの場合は回転量に制限があるのでここで補正しておく
          env_.task->sendMsgAll(MSG_GAME_CONTROL_STOP);
          Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
          ResetCamera *cam = static_cast<ResetCamera *>(t.get());
          cam->type(QUAD_INOUT);
          cam->time(start_delay_ * 0.8);
          cam->distance(env_.camera->getDist());

          Vec2<float> rot = QuatToAngle(env_.camera->getRot());
          float rx_max = env_.camera->pitch_max();
          if (rot.x > rx_max) rot.x = rx_max;
          if (rot.x < -rx_max) rot.x = -rx_max;
          cam->rotate(rot);
        }
        HideGameCenerBtn();
        HideTweetBtn();
      }
      break;

    case MSG_TITLE_EARTH_TEX_CHANGE:
      {
        env_.earth->texture(env_.earth_texture);
        double value = env_.earth_texture;
        env_.settings->set<double>("texture", value);
      }
      break;

    case MSG_TITLE_END:
      {
        bool firstexec = false;
        env_.settings->set<bool>("firstexec", firstexec);
      }
      break;
    }
  }
  
};

}
