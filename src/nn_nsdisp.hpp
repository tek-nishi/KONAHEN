
#pragma once

//
// 南北の極を表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"


namespace ngs {

class NsDisp : public TaskProc {
  GameEnv& env_;
  bool active_;

  const Camera& camera_;
  const Camera& cockpit_;
  const Earth& earth_;
  const TexMng::tex_ptr texture_;

  picojson::object& params_;
  const float radius_;
  const float scale_;

  Vec2<float> n_size_;
  Vec2<float> n_uv_;
  Vec2<float> s_size_;
  Vec2<float> s_uv_;
  
  float time_, time_ed_;
  float alpha_;
  float alpha_st_, alpha_ed_;

  bool fin_exec_;

public:
  explicit NsDisp(GameEnv& env) :
    env_(env),
    active_(true),
    camera_(*env.camera),
    cockpit_(*env.cockpit),
    earth_(*env.earth),
    texture_(env.texMng->read(*env.path + "devdata/game.png")),
    params_(env.params->value().get<picojson::object>()["nsdisp"].get<picojson::object>()),
    radius_(env.earth->getRadius()),
    scale_(params_["scale"].get<double>()),
    time_(),
    time_ed_(2.0),
    alpha_(),
    alpha_st_(),
    alpha_ed_(1.0),
    fin_exec_()
  {
    DOUT << "NsDisp()" << std::endl;

    {
      picojson::object& object = params_[env.localize->get("n_widget")].get<picojson::object>();
      n_size_.set(object["width"].get<double>(), object["height"].get<double>());
      n_uv_.set(object["u"].get<double>(), object["v"].get<double>());
      // N極の表示情報
    }
    
    {
      picojson::object& object = params_[env.localize->get("s_widget")].get<picojson::object>();
      s_size_.set(object["width"].get<double>(), object["height"].get<double>());
      s_uv_.set(object["u"].get<double>(), object["v"].get<double>());
      // S極の表示情報
    }
  }

  ~NsDisp()
  {
    DOUT << "~NsDisp()" << std::endl;
  }
  
  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (time_ < time_ed_)
    {
      time_ += delta_time;
      Easing easing;
      easing.ease(alpha_, time_, alpha_st_, alpha_ed_, time_ed_);

      if (fin_exec_)
      {
        bool active = (time_ < time_ed_);
        if (!active) active_ = false;
      }
    }
  }

  void draw()
  {
    Vec3<float> center = camera_.posToScreen(Vec3<float>(0, 0, 0), earth_.mtx());

    Vec3<float> pos = camera_.posToScreen(Vec3<float>(0, radius_ * scale_, 0), earth_.mtx());
    Vec3<float> d = pos - center;
    d.z *= 100000.0f;
    d.unit();
    float r = d.angle(Vec3<float>(0, 0, -1));
    float alpha = 1.0;
    if (r > (PI / 2.0f))
    {
      alpha = cos(r - (PI / 2.0f));
    }

    static const Matrix mat;                        // FIXME:単位行列が欲しいだけ
    pos = cockpit_.posToWorld(Vec2<float>(pos.x, pos.y), 0, mat.value());
    // ↑3D座標を表示座標に変換する

    GrpSprite obj;
    obj.pos(pos.x, pos.y);
    obj.size(n_size_);
    obj.center();
    obj.texture(texture_);
    obj.uv(n_uv_);
    obj.col(1, 1, 1, alpha * alpha_);
    obj.draw();
    // N極
    
    pos = camera_.posToScreen(Vec3<float>(0, -radius_ * scale_, 0), earth_.mtx());
    d = pos - center;
    d.z *= 100000.0f;
    d.unit();
    r = d.angle(Vec3<float>(0, 0, -1));
    alpha = 1.0f;
    if (r > (PI / 2.0f))
    {
      alpha = cos(r - (PI / 2.0f));
    }

    pos = cockpit_.posToWorld(Vec2<float>(pos.x, pos.y), 0, mat.value());

    obj.pos(pos.x, pos.y);
    obj.size(s_size_);
    obj.center();
    obj.texture(texture_);
    obj.uv(s_uv_);
    obj.col(1, 1, 1, alpha * alpha_);
    obj.draw();
    // S極
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

    case MSG_GAME_END:
      if (!fin_exec_)
      {
        fin_exec_ = true;
        time_ = 0.0f;
        alpha_st_ = alpha_;
        alpha_ed_ = 0.0f;
      }
      break;
    }
  }
  
};


}
