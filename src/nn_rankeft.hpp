
#pragma once

//
// ランク決定時のエフェクト
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class RankEft : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;
  const float eft_time_;
  const float eft_speed_;

  struct Particle {
    Vec2<float> pos;
    Vec2<float> startPos;
    Vec2<float> endPos;
    float alpha;
    WidgetsMap::WidgetPtr widget;
    float time_ed;
  };
  std::vector<Particle> particle_;

  float time_;
  

public:
  explicit RankEft(GameEnv& env, const Vec2<float>& center) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["rankeft"].get<picojson::object>()),
    eft_time_(params_["eft_time"].get<double>()),
    eft_speed_(params_["eft_speed"].get<double>()),
    time_()
  {
    DOUT << "RankEft()" << std::endl;

    const WidgetsMap widgets(params_["widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
    u_int num = widgets.size();

    u_int rank = env_.rank;
    u_int eft_num = (rank + 1) * 4;
    // ランクが高いとエフェクトの数も多い
    
    for (u_int i = 0; i < eft_num; ++i)
    {
      Particle particle;
      float v = eft_speed_ + rand() % 50;
      Vec2<float> vec(v, 0);
      float r = (float)(rand() % 360) * PI / 180.0f;
      vec.rotate(r);
      // 移動先座標を適当に決める

      std::stringstream sstr;
      sstr << (1 + rand() % num);
      particle.widget = widgets.get(sstr.str());
      // 用意されたパターンから適当に選ぶ
      
      particle.pos = center + vec * 0.4f;
      particle.startPos = center + vec * 0.4f;
      particle.endPos = vec + particle.startPos;
      particle.alpha = 1.0f;
      particle.widget->blend_mode(GRP_BLEND_ADD);
      particle.time_ed = eft_time_ + (rand() % 5) * 0.15f;
      
      particle_.push_back(particle);
    }
  }
  
  ~RankEft()
  {
    DOUT << "~RankEft()" << std::endl;
  }

  bool active() const { return active_; }

  void step(const float delta_time)
  {
    time_ += delta_time;

    bool exec = false;
    Easing easing;
    for (std::vector<Particle>::iterator it = particle_.begin(); it != particle_.end(); ++it)
    {
      easing.ease(it->pos, time_, it->startPos, it->endPos, it->time_ed, QUART_OUT);
      easing.ease(it->alpha, time_, 1.0f, 0.0f, it->time_ed);
      if (time_ < it->time_ed) exec = true;
      // 全ての計算が終わったらタスクを止める措置
    }
    active_ = exec;
  }

  void draw()
  {
    for (std::vector<Particle>::iterator it = particle_.begin(); it != particle_.end(); ++it)
    {
      if (time_ < it->time_ed)
      {
        float alpha = it->alpha * (float)(5 + (rand() % 6)) * 0.1f;
        // 若干キラキラさせる
        
        it->widget->setCol(1, 1, 1, alpha);
        it->widget->setPos(it->pos);
        it->widget->draw();
      }
    }
  }

  void msg(const int msg)
  {
    switch(msg)
    {
    case MSG_GAME_RESULT_END:
      if (active_)
      {
        active_ = false;
      }
      break;
    }
  }
};

}
