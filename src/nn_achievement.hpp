
#pragma once

//
// 実績解除画面
//

#include "nn_agree.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

//
// ユーティリティー関数群
//

// 隠し要素などのセーブデータを初期化する
void setupAchievement(const GameEnv& env)
{
  picojson::object& object = env.settings->json().value().get<picojson::object>();
  if (!object["achievement"].is<picojson::object>())
  {
    object["achievement"] = picojson::value(picojson::object());
    // 初回起動時は新規作成
  }

  picojson::object& params = object["achievement"].get<picojson::object>();
  const picojson::object& lists = env.params->value().get<picojson::object>()["achievement"].get<picojson::object>()["text"].get<picojson::object>();
  for (picojson::object::const_iterator it = lists.begin(); it != lists.end(); ++it)
  {
    if (!object[it->first].is<bool>())
    {
      object[it->first] = picojson::value(false);
      params[it->first] = picojson::value(false);
      // バージョンが上がって追加される場合もあるので、毎回チェックする
    }
  }

  if (!object["places"].is<picojson::object>())
  {
    picojson::object places;
    const picojson::object& lists = env.places->value().get<picojson::object>();
    for (picojson::object::const_iterator it = lists.begin(); it != lists.end(); ++it)
    {
      places[it->first] = picojson::value(0.0);
    }
    object["places"] = picojson::value(places);
    // 問題ごとの正解数
  }
}

bool isAchievementUnlock(const std::string& name, const GameEnv& env, const bool lite = false)
{
#ifdef VER_LITE
  if (!lite) return false;
#endif
  return env.settings->get<bool>(name);
}

void AchievementUnlock(const std::string& name, GameEnv& env, const bool lite = false)
{
#ifdef VER_LITE
  if (!lite) return;
#endif
  bool value = true;
  env.settings->set<bool>(name, value);
}

void AchievementLock(const std::string& name, GameEnv& env)
{
  bool value = false;
  env.settings->set<bool>(name, value);
  picojson::object& achievs = env.settings->get<picojson::object>("achievement");
  achievs[name] = picojson::value(false);
}


// 問題の正解数をカウント
void PlaceHit(const std::string& name, GameEnv& env)
{
  picojson::object& places = env.settings->json().value().get<picojson::object>()["places"].get<picojson::object>();
  float num = 1;
  if (places[name].is<double>()) num += places[name].get<double>();
  places[name] = picojson::value(num);
}

// 問題の正解数を返す
int PlaceCorrectNum(const std::string& name, GameEnv& env)
{
  picojson::object& places = env.settings->json().value().get<picojson::object>()["places"].get<picojson::object>();
  if (!places[name].is<double>())
  {
    places[name] = picojson::value(0.0);
    // データが存在しない場合は作成
  }
  return places[name].get<double>();
}

// 全問題の正解率をカウント
float PlaceCorrectRate(GameEnv& env)
{
  const picojson::object& places_data = env.places->value().get<picojson::object>();
  picojson::object& places = env.settings->json().value().get<picojson::object>()["places"].get<picojson::object>();
  int num = 0;
  for (picojson::object::const_iterator it = places_data.begin(); it != places_data.end(); ++it)
  {
    if (!places[it->first].is<double>())
    {
      places[it->first] = picojson::value(0.0);
      // データが存在しない場合は作成
    }
    else
    {
      if (places[it->first].get<double>() > 0.0) ++num;
    }
  }

  DOUT << "correct:" << num << ":" << places_data.size() << std::endl;
  
  return (float)num / (float)places_data.size();
}


class Achievement: public TaskProc
{
  GameEnv& env_;
  const Localize& localize_;
  bool active_;

  picojson::object& params_;

  const TexMng::tex_ptr texture_;
  FntMng::FontPtr font_;
  int height_;

  const std::string& title_;
  std::vector<std::string> texts_;

  float delay_;
  bool draw_;
  
  float time_, time_ed_;
  Vec2<float> vct_;
  Vec2<float> vct_st_, vct_ed_;
  int ease_type_;

  float agree_delay_;

  bool fin_exec_;
  bool touched_;

  struct Disp {
    const std::string *text;
    float x, y;
  };

public:
  explicit Achievement(GameEnv& env) :
    env_(env),
    localize_(*env.localize),
    active_(true),
    texture_(env.texMng->read(*env.path + "devdata/round.png")),
    params_(env.params->value().get<picojson::object>()["achievement"].get<picojson::object>()),
    font_(ReadFont("achievement", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
    height_(font_->height()),
    title_(localize_.get(params_["title"].get<std::string>())),
    delay_(params_["delay"].get<double>()),
    draw_(),
    time_(),
    time_ed_(0.5),
    vct_st_(0, 0.7),
    vct_ed_(1, 1),
    ease_type_(CUBIC_OUT),
    agree_delay_(1.5),
    fin_exec_(),
    touched_()
  {
    DOUT << "Achievement()" << std::endl;

    const picojson::object& achievs = env_.settings->get<picojson::object>("achievement");
#ifdef _DEBUG
    if (env_.keyinp->press('u'))
    {
      DOUT << "All Achievement Unlock!" << std::endl;
      for (picojson::object::const_iterator it = achievs.begin(); it != achievs.end(); ++it)
      {
        AchievementUnlock(it->first, env_);
        // 全解除
      }
    }
#endif

    for (picojson::object::const_iterator it = achievs.begin(); it != achievs.end(); ++it)
    {
      if (!it->second.get<bool>() && env_.settings->get<bool>(it->first))
      {
        texts_.push_back(it->first);
      }
    }

    if (texts_.empty())
    {
      delay_ = 0;
      fin_exec_ = true;
      time_ = 0;
      time_ed_ = 0;
    }
  }
  
  ~Achievement()
  {
    DOUT << "~Achievement()" << std::endl;

    picojson::object& achievs = env_.settings->get<picojson::object>("achievement");
    for (picojson::object::iterator it = achievs.begin(); it != achievs.end(); ++it)
    {
      it->second = picojson::value(env_.settings->get<bool>(it->first));
    }
    // フラグの後始末
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (delay_ > 0.0)
    {
      draw_ = (delay_ -= delta_time) <= 0.0;
      if (!draw_) return;
      env_.sound->play("unlock");
    }
    
    if (time_ < time_ed_) time_ += delta_time;
    Easing easing;
    easing.ease(vct_, time_, vct_st_, vct_ed_, time_ed_, ease_type_);

    if (agree_delay_ > 0)
    {
      if ((agree_delay_ -= delta_time) <= 0.0)
      {
        env_.task->add<Agree>(TASK_PRIO_2D, env_);
      }
    }

    if (fin_exec_ && (time_ >= time_ed_))
    {
      active_ = false;
      env_.task->sendMsgAll(MSG_ACHIEVEMENT_END);
    }
    else
    if (touched_)
    {
      touched_ = false;
      time_ = 0;
      time_ed_ = 0.6;
      vct_st_ = vct_;
      vct_ed_.set(0, 0.8);
      ease_type_ = CUBIC_OUT;
      fin_exec_ = true;
    }
  }

  void draw()
  {
    if (!draw_) return;

    glPushMatrix();
    glScalef(vct_.y, vct_.y, 1.0);

    font_->col(1, 1, 1, vct_.x);

    std::vector<Disp> disp;
    // 事前に表示エリアを計算するために必要

    const Vec2<float>& size = font_->size(title_);
    float y = 0.0;
    {
      Disp d = { &title_, size.x / -2, y };
      disp.push_back(d);
    }
    
    Vec2<float> bg_size(size.x, size.y + 2);

    y += height_ * 3;
    picojson::object& achievs = params_["text"].get<picojson::object>();
    for (std::vector<std::string>::const_iterator it = texts_.begin(); it != texts_.end(); ++it)
    {
      picojson::array& array = achievs[*it].get<picojson::array>();
      for(picojson::array::iterator it = array.begin(); it != array.end(); ++it)
      {
        const std::string& text = localize_.get(it->get<std::string>());
        const Vec2<float>& size = font_->size(text);
        float x = size.x / -2.0;
        
        Disp d = { &text, x, y };
        disp.push_back(d);

        if (size.x > bg_size.x)
        {
          bg_size.x = size.x;
        }
        bg_size.y = y + 2;
        y += height_ + 10;
      }
      y += 10;
    }

    GrpRoundBox obj;
    obj.pos(-bg_size.x / 2.0 - 25.0, -bg_size.y / 2.0 - 25 - 15);
    obj.size(bg_size.x + 50.0, bg_size.y + 50.0);
    obj.texture(texture_);
    obj.col(0, 0, 0, 0.5 * vct_.x);
    obj.draw();

    for (std::vector<Disp>::iterator it = disp.begin(); it != disp.end(); ++it)
    {
      font_->pos(it->x, it->y - bg_size.y / 2.0 + height_ / 2.0 - 1 - 15);
      font_->draw(*it->text);
    }
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
