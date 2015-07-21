
#pragma once

//
// ゲーム本編
//

#include <iostream>
#include <string>
#if defined (_MSC_VER)
#include <random>
#else
#include <tr1/random>
#endif
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_place.hpp"
#include "nn_gameenv.hpp"
#include "nn_konahen.hpp"
#include "nn_touchjust.hpp"
#include "nn_results.hpp"
#include "nn_score.hpp"
#include "nn_placedisp.hpp"
#include "nn_gamemenu.hpp"
#include "nn_gameintro.hpp"
#include "nn_gamestart.hpp"
#include "nn_gameover.hpp"
#include "nn_gameinfo.hpp"
#include "nn_playlevel.hpp"
#include "nn_playkonahen.hpp"
#include "nn_playtime.hpp"
#include "nn_results.hpp"
#include "nn_ranking.hpp"
#include "nn_anstime.hpp"
#include "nn_widgets_map.hpp"
#include "nn_achievement.hpp"
#include "nn_distance.hpp"
#include "nn_nsdisp.hpp"
#include "nn_equator.hpp"
#include "nn_replay.hpp"
#include "nn_hintcircle.hpp"
#include "Twitter.h"


namespace ngs {

class GameMain : public TaskProc
{
  GameEnv& env_;
  Replay& replay_;
  bool active_;

  picojson::object& params_;
  picojson::object& places_;

  const WidgetsMap widgets_;
  
  bool pause_;
  bool q_started_;
  bool playing_;
  bool q_refresh_;
  size_t q_index_;
  float q_refresh_delay_;
  float q_refresh_delay_param_;
  int level_ofs_;
  bool touch_miss_;
  int tap_num_;
  float q_time_;

  int time_sec_;

  float rate_;
  bool time_remain_;

  float start_time_;
  float bonus_time_;
  float extend_rate_;

  float touch_radius_;

  std::vector<std::string> place_names_;
  Place *place_;
  std::string q_name_;
  std::vector<std::string> answered_;

  unsigned int rseed_;
  std::tr1::mt19937 mt_;

#ifdef ANY_CORRECT
  bool any_correct_;
#endif

  // 問題ソート用の関数オブジェクト
  struct PlaceSort {
    const std::map<std::string, int>& correct_;
    PlaceSort(std::map<std::string, int>& correct) :
      correct_(correct)
    {}
    
    bool operator()(const std::string& left, const std::string& right) const
    {
      std::map<std::string, int>::const_iterator l_it = correct_.find(left);
      int l_num = l_it->second;

      std::map<std::string, int>::const_iterator r_it = correct_.find(right);
      int r_num = r_it->second;
      
      return l_num < r_num;
    }
  };
  
  void createPlace(const int level)
  {
    std::map<std::string, int> correctNum;
    place_names_.clear();
    for(picojson::object::iterator it = places_.begin(); it != places_.end(); ++it)
    {
      picojson::object& obj = (it->second).get<picojson::object>();
      if (obj["level"].get<double>() == (level + level_ofs_))
      {
        place_names_.push_back(it->first);
        correctNum.insert(std::map<std::string, int>::value_type(it->first, PlaceCorrectNum(it->first, env_) + rand() % 3));
      }
    }
    random_shuffle_for_engine(place_names_.begin(), place_names_.end(), mt_);
#ifndef REPLAY_REC
    if (!env_.demo)
    {
      std::sort(place_names_.begin(), place_names_.end(), PlaceSort(correctNum));
      // DEMOでなければ正解数＋αでなんとなくソート
    }
#endif
    // １レベルぶんの問題リストを作成

#ifdef _DEBUG
    for (std::vector<std::string>::iterator it = place_names_.begin(); it != place_names_.end(); ++it)
    {
      DOUT << *it << ":" << correctNum[*it] << std::endl;
    }
#endif
    
    {
      picojson::array& array = params_["question"].get<picojson::array>();
      env_.question = array[level + level_ofs_].get<double>();
      // 問題数
    }
    {
      picojson::array& array = params_["ans_time"].get<picojson::array>();
      env_.ans_time_max = array[level].get<double>();
      // 解答時間
    }
    {
      picojson::array& array = params_["q_refresh_delay"].get<picojson::array>();
      q_refresh_delay_param_ = array[level].get<double>();
      // 問題切り替え時間
    }
    env_.konahen_cur = 0;
  }
  
  void readPlace(const std::string& name)
  {
    picojson::object& place = places_[name].get<picojson::object>();
    std::map<std::string, Place>::iterator it = env_.place_data->find(name);
#ifdef NO_READ_PLACES
    if (it == env_.place_data->end())
    {
      std::string file = *env_.path + "devdata/place/" + place["file"].get<std::string>();
      Place place(file);
      env_.place_data->insert(std::map<std::string, Place>::value_type(name, place));
      it = env_.place_data->find(name);
      DOUT << "Read:" << name << std::endl;
    }
#endif
    place_ = &it->second;

    env_.cur_place = place["name"].get<std::string>();
    env_.answer = place["answer"].is<std::string>();
    if (env_.answer) env_.ans_place = place["answer"].get<std::string>();
    env_.onetime = place["onetime"].is<bool>() ? place["onetime"].get<bool>() : false;

    std::vector<Vec2<float> > center;
    place_->center(center);                         // 各範囲の中心を求める
    env_.place_center.clear();
    for(std::vector<Vec2<float> >::iterator it = center.begin(); it != center.end(); ++it)
    {
      Vec3<float> pos = locToPos(*it);
      env_.place_center.push_back(pos);
    }
    place_->radius(touch_radius_);                  // 判定の"のりしろ"を設定
  }

  void lvupExec()
  {
    picojson::array& array = params_["question"].get<picojson::array>();
    int question = array[env_.level + level_ofs_ + 1].get<double>();
    if (question < 0)
    {
      // 問題が無い場合はゲーム終了
      env_.ans_time = 0.0;
      env_.task->sendMsgAll(MSG_GAME_END);
      env_.no_question = true;
      return;
    }

    this->createPlace(++env_.level);
    q_index_ = 0;

    if (env_.time >= 0.0)
    {
      float dt = start_time_ - env_.time;
      float bonus = (bonus_time_ - dt) * extend_rate_;
      if (bonus > 0.0)
      {
        env_.time += bonus;
        if (env_.time > 99.9) env_.time = 99.9;
      }
      start_time_ = env_.time;
      DOUT << "Time extend:" << bonus << std::endl;
    }
    env_.sound->play("lvup", 0.8, 0.25);
    env_.task->sendMsgAll(MSG_GAME_LEVELUP);    
  }

  void ansDisp(const bool se = true)
  {
    // 時間切れの場合に答えを教える
    Task::ProcPtr t = env_.task->add<ResetCamera>(TASK_PRIO_3D, env_);
    ResetCamera *cam = static_cast<ResetCamera *>(t.get());
    cam->type(QUAD_INOUT);
    cam->time(1.0);

    if (se) env_.sound->play("miss");

    const std::vector<Vec3<float> >& center = env_.place_center;
    Vec3<float> pos = center[0];

    env_.hit_pos = pos;
    env_.task->sendMsgAll(MSG_GAME_TOUCH_ANS);

    const float ry = env_.earth->rotate();
#if 1
    // オイラー角っぽく正解位置を設定
    Vec2<float> rot = posToLoc(pos);
    std::swap(rot.x, rot.y);
    float rx_max = env_.camera->pitch_max();
    if (rot.x > rx_max) rot.x = rx_max;
    if (rot.x < -rx_max) rot.x = -rx_max;
    rot.y = PI / 2.0 - rot.y - Ang2Rad(ry);
    cam->rotate(rot);

    float dist = env_.camera->getDist();
    if (dist < 1950.0) dist = 1950.0;
    cam->distance(dist);
#else
    // クオータニオンで正解位置を設定
    Matrix mat;
    mat.rotateY(Ang2Rad(ry));
    mat.apply(pos);
    cam->rotate(pos);
    cam->distance(env_.camera->getDist());
#endif
  }

  bool isGameOver(const int mode, const bool refresh)
  {
    bool no_q = q_refresh_ && (q_index_ == place_names_.size());
    switch (mode)
    {
    case GAME_MODE_SURVIVAL:
      return refresh || no_q;
      // サバイバル:出題時間切れで次の出題→終了
      
    default:
      return (env_.time == 0) || no_q;
      // 通常:ゲーム時間切れ→終了
    }
  }
  
public:
  GameMain(GameEnv& env, Replay& replay) :
    env_(env),
    replay_(replay),
    active_(true),
    params_(env.params->value().get<picojson::object>()["game"].get<picojson::object>()),
    places_(env.places->value().get<picojson::object>()),
    place_(),
    widgets_(params_["widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom),
    pause_(),
    q_started_(),
    playing_(),
    q_refresh_(),
    q_index_(),
    q_refresh_delay_(),
    level_ofs_(),
    touch_miss_(),
    tap_num_(),
    q_time_(),
    rate_(1.0),
    time_remain_((env.game_mode == GAME_MODE_SURVIVAL) ? true : false),
    touch_radius_(params_["touch_radius"].get<double>()),
    rseed_((env.demo ? replay_.rseed() : time(0)) & 0x7fffffff), // long型が64bitの環境がありえるのでこうしとく
    mt_(static_cast<unsigned long>(rseed_))
  {
    DOUT << "GameMain()" << std::endl;

    {
      picojson::array& array = params_["level_ofs"].get<picojson::array>();
      level_ofs_ = array[env_.game_mode].get<double>();
    }
    {
      picojson::array& array = params_["time"].get<picojson::array>();
      env_.time = array[env_.game_mode].get<double>();
      start_time_ = env_.time;
      bonus_time_ = env_.time;
    }
    {
      picojson::array& array = params_["extend_rate"].get<picojson::array>();
      extend_rate_ = array[env_.game_mode].get<double>();
    }

    env_.ans_time     = 0;
    env_.ans_time_max = 0;
    env_.level        = 0;
    env_.konahen      = 0;
    env_.just         = 0;
    env_.just_total   = 0;
    env_.just_max     = 0;
    env_.tap_num      = 0;
    env_.miss_num     = 0;
    env_.total_time   = 0;
    env_.score        = 0;
    env_.rank         = 0;
    env_.q_time.clear();

    env_.question       = 0;
    env_.question_total = 0;
    env_.konahen_cur    = 0;
    env_.started        = false;
    env_.no_question    = false;

    env_.answer = false;
    env_.rseed  = rseed_;

    time_sec_ = env_.time;

    this->createPlace(env_.level);

    if (env_.game_mode == GAME_MODE_NORMAL)
    {
      env_.task->add<Equator>(TASK_PRIO_3D, env_);
      env_.task->add<NsDisp>(TASK_PRIO_2D, env_);
    }
    if (env.game_mode == GAME_MODE_NORMAL) env_.task->add<HintCircle>(TASK_PRIO_3D, env_);

    std::string combo_eft("combo_eft");
    env_.task->add<Konahen>(TASK_PRIO_3D, env_, combo_eft);
    std::string konahen_eft("konahen_eft");
    env_.task->add<Konahen>(TASK_PRIO_3D, env_, konahen_eft);
    std::string just_eft("just_eft");
    env_.task->add<Konahen>(TASK_PRIO_3D, env_, just_eft);
    std::string miss_eft("miss_eft");
    env_.task->add<Konahen>(TASK_PRIO_3D, env_, miss_eft);
    //↑各種こなへん演出
    
    env_.task->add<GameInfo>(TASK_PRIO_2D, env_, widgets_);
    env_.task->add<Distance>(TASK_PRIO_2D, env_, widgets_);
    env_.task->add<PlaceDisp>(TASK_PRIO_2D, env_);
    env_.task->add<PlayLevel>(TASK_PRIO_2D, env_, widgets_);
    env_.task->add<PlayKonahen>(TASK_PRIO_2D, env_, widgets_);
    env_.task->add<AnsTime>(TASK_PRIO_2D, env_, widgets_);
    if (env_.time > 0) env_.task->add<PlayTime>(TASK_PRIO_2D, env_, widgets_);
    env_.task->add<GameIntro>(TASK_PRIO_2D, env_);
    
    env_.earth->setRotSpeed(0.0, 2.0, CIRC_IN);

    env_.task->sendMsgAll(MSG_GAME_CONTROL_START);
    env_.task->sendMsgAll((env.game_mode == GAME_MODE_NORMAL) ? MSG_CONTROL_XY : MSG_CONTROL_MIX);
    env_.task->sendMsgAll(MSG_GAME_MAIN_INIT);
    env_.task->sendMsgAll(MSG_GAME_TOUCH_BLINK_OFF);
    // 場所点滅表示しない

#ifdef ANY_CORRECT
    any_correct_ = false;
    // どこでも正解モード
#endif
  }

  ~GameMain()
  {
    DOUT << "~GameMain()" << std::endl;
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (pause_ || !q_started_ || !playing_) return;

    if (env_.time > 0.0)
    {
      if ((env_.time -= delta_time) < 0.0) env_.time = 0.0;
    }

    bool refresh = false;
    if (env_.ans_time > 0.0)
    {
      refresh = (env_.ans_time -= delta_time) <= 0.0;
      q_time_ += delta_time;

      float rate = (env_.ans_time_max > 0) ? (env_.ans_time / env_.ans_time_max) : 1;
      if (rate_ > 0.25 && rate <= 0.25)
      {
        env_.task->sendMsgAll(MSG_GAME_HURRYUP);
      }
      rate_ = rate;
      
      if (refresh)
      {
        env_.task->sendMsgAll(MSG_GAME_CONTROL_STOP);
        this->ansDisp();
        touch_miss_ = true;
        env_.just = 0;
        env_.miss_num += 1;
        env_.tap_num += tap_num_;
        env_.total_time += q_time_;
        
        // q_refresh_ = false;
        q_refresh_delay_ = 1.5;
        if ((env_.time > 5.0) && ((rand() % 100) < 40)) env_.task->sendMsgAll(MSG_GAME_DOBEST);
      }
    }
    
    if (q_refresh_delay_ > 0.0)
    {
      q_refresh_ = (q_refresh_delay_ -= delta_time) <= 0.0;
    }
    
    if (this->isGameOver(env_.game_mode, refresh))
    {
      env_.task->sendMsgAll(MSG_GAME_END);
    }
    else
    if (q_refresh_)
    {
      q_name_ = place_names_[q_index_];
      this->readPlace(q_name_);

      env_.sound->play("question", 1.0, 0.05f);
      env_.task->sendMsgAll(MSG_GAME_PLACEDISP_START);
      if (env_.onetime) env_.task->sendMsgAll(MSG_GAME_PLACEDISP_ANS);
      env_.task->sendMsgAll(MSG_GAME_CONTROL_START);
      env_.task->sendMsgAll(MSG_GAME_TOUCH_START);

      env_.ans_time = env_.ans_time_max;
      rate_ = 1.0;

      tap_num_ = 0;
      q_time_ = 0.0f;

      env_.started = true;
      ++q_index_;
      ++env_.question_total;
      q_refresh_ = false;
    }

    int second = env_.time;
    if (second < 10 && second != time_sec_)
    {
      time_sec_ = second;
      env_.sound->play("count_down");
    }
    if ((second < 10) && !time_remain_)
    {
      env_.task->sendMsgAll(MSG_GAME_TIMESHORT);
      time_remain_ = true;
    }

#ifdef ANY_CORRECT
    if (env_.keyinp->get() == 'C')
    {
      any_correct_ = !any_correct_;
      DOUT << "Correct Mode:" << any_correct_ << std::endl;
    }
#endif
#ifdef _DEBUG
    if (env_.keyinp->get() == 'Q')
    {
      env_.time = delta_time;
      // 強制ゲームオーバー
    }
#endif
  }
  
  void draw() {}

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_START:
      {
        q_started_ = true;
        q_refresh_ = true;
        playing_ = true;
        env_.task->add<GameMenu>(TASK_PRIO_2D, env_);
        if (env_.demo)
        {
          replay_.replay(true);
        }
        else
        {
#ifdef REPLAY_REC
          replay_.record(true);
#endif
        }
      }
      break;

    case MSG_GAME_END:
      {
        if (env_.ans_time > 0.0)
        {
          this->ansDisp(false);
          // env_.miss_num += 1;
          // オマケでミスにはカウントしない
          env_.tap_num += tap_num_;
          env_.total_time += q_time_;
        }
        
        RankInfo info = {
          params_,
          env_.konahen,
          env_.just_total,
          env_.just_max,
          env_.miss_num,
          static_cast<float>(env_.tap_num),
          env_.total_time
        };
        env_.rank = ScoreToRank(info);

        DOUT <<
          " Konahen:" << env_.konahen <<
          " Level:" << env_.level <<
          " Play Time:" << env_.total_time <<
          " Just Total:" << env_.just_total <<
          " Just Max:" << env_.just_max <<
          " Miss:" << env_.miss_num <<
          " Tap:" << env_.tap_num <<
          " Score:" << env_.score <<
          " Rank:" << env_.rank <<
          std::endl;

        for(std::vector<float>::iterator it = env_.q_time.begin(); it != env_.q_time.end(); ++it)
        {
          DOUT << *it << std::endl;
        }

        q_refresh_ = false;
        q_refresh_delay_ = 0;
        env_.ans_time = 0;
        playing_ = false;
        env_.sound->play(env_.no_question ? "complete" : "stop");
        env_.task->sendMsgAll(MSG_GAME_TOUCH_STOP);
        env_.task->add<GameOver>(TASK_PRIO_2D, env_, widgets_);

        if (env_.demo)
        {
          replay_.replay(false);
        }
        else
        {
#ifdef REPLAY_REC
          replay_.record(false);
          replay_.rseed() = env_.rseed;
          replay_.write();
#endif
        }
      }
      break;

    case MSG_GAME_INTRO_END:
      {
        env_.task->add<GameStart>(TASK_PRIO_2D, env_, widgets_);
      }
      break;

    case MSG_GAME_GAMEOVER_END:
      // DEMOの時は結果画面へ移行しない
      if (!env_.demo)
      {
        for (std::vector<std::string>::const_iterator it = answered_.begin(); it != answered_.end(); ++it)
        {
          PlaceHit(*it, env_);
          // 正解した問題を記録
        }
        env_.task->add<Results>(TASK_PRIO_2D, env_, widgets_);
      }
      break;

    case MSG_GAME_RESULT_END:
      {
        bool record = true;
        env_.task->add<Ranking>(TASK_PRIO_2D, env_, env_.game_mode, record);
      }
      break;
      
    case MSG_RANKING_END:
      {
        env_.cleanup = true;
      }
      break;

    case MSG_GAME_CLEANUP:
      {
        if (env_.demo)
        {
          replay_.replay(false);
        }
        else
        {
          replay_.record(false);
        }
        active_ = false;
      }
      break;

    case MSG_GAME_PAUSE:
      {
        pause_ = !pause_;
      }
      break;
      
    case MSG_GAME_TOUCH:
      if (place_)
      {
        bool correct = place_->hit(locToPos2d(posToLoc(env_.hit_pos)));
#ifdef ANY_CORRECT
        correct = any_correct_ ? true : correct;
#endif
        env_.correct = correct;
        ++tap_num_;
        if (correct)
        {
          env_.task->sendMsgAll(MSG_GAME_TOUCH_STOP);
          env_.tap_num += tap_num_;
          env_.q_time.push_back(q_time_);
          env_.total_time += q_time_;

          const char *se_tbl[] = {
            "touch_just1",
            "touch_just2",
            "touch_just3",
            "touch_just4"
          };
          int idx = env_.just < 3 ? env_.just : 3;
          env_.sound->play(touch_miss_ ? "touch_in" : se_tbl[idx]);

          answered_.push_back(q_name_);
          // 正解した問題を記録

          ++env_.konahen;
          ++env_.konahen_cur;
          
          if (!touch_miss_)
          {
            ++env_.just;
            ++env_.just_total;
            if (env_.just_max < (env_.just - 1)) env_.just_max = (env_.just - 1);

            TouchJustParam param = {
              env_.texMng,
              env_.just,
              env_.params->value().get<picojson::object>(),
              *env_.path
            };
            env_.task->add<TouchJust>(TASK_PRIO_2D, param, widgets_);
          }
          touch_miss_ = false;

          ScoreKonahen(env_);

          env_.task->sendMsgAll(MSG_GAME_TOUCH_IN);
          if (env_.konahen_cur == env_.question)
          {
            this->lvupExec();
          }

          q_refresh_delay_ = q_refresh_delay_param_;
          env_.ans_time = 0;
        }
        else
        {
          env_.task->sendMsgAll(MSG_GAME_TOUCH_OUT);
          env_.sound->play("touch_out");
          touch_miss_ = true;
          env_.just = 0;
        }
      }
      break;
    }
  }
};

}
