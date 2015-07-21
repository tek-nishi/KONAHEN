
#pragma once

//
// カメラのリセット
//

#include "nn_gameenv.hpp"


namespace ngs {

class ResetCamera : public TaskProc
{
  GameEnv& env_;
  bool active_;

  picojson::object& params_;

  Camera& camera_;

  float type_;
  float dist_st_;
  float dist_ed_, dist_ed_org_;
  float time_cur_, time_ed_;

  Quat<float> rot_st_;
  Quat<float> rot_ed_;

  float delay_;
  bool slerp_;

  void finish()
  {
    camera_.setRot(rot_ed_);
    camera_.setDist(dist_ed_);
    env_.task->sendMsgAll(MSG_RESETCAMERA_STOP);
    active_ = false;
  }
  
public:
  explicit ResetCamera(GameEnv& env) :
    env_(env),
    active_(true),
    params_(env.params->value().get<picojson::object>()["reset_camera"].get<picojson::object>()),
    camera_(*env.camera),
    type_(LINEAR),
    dist_st_(camera_.getDist()),
    time_cur_(),
    time_ed_(1.0f),
    rot_st_(camera_.getRot()),
    delay_()
  {
    DOUT << "ResetCamera()" << std::endl;

    rot_st_.unit();                                 // TIPS:正規化しないとぐるぐる回る
    rot_ed_ = rot_st_;
    
    picojson::object& params = env.params->value().get<picojson::object>();
    dist_ed_ = params["camera"].get<picojson::object>()["dist"].get<double>();
    dist_ed_org_ = dist_ed_;

    camera_.setDist(dist_st_);
  }
  
  ~ResetCamera()
  {
    DOUT << "~ResetCamera()" << std::endl;
  }

  bool active() const { return active_; }

  void type(const int type) { type_ = type; }
  void time(const float time) { time_ed_ = time; }
  void delay(const float delay) { delay_ = delay; }

  // 地球上の座標から向きを求める
  void rotate(const Vec3<float>& target)
  {
    Vec3<float> v1(0, 0, 1);
    Vec3<float> v2(target);
    v2.unit();
    Vec3<float> cross = v2.cross(v1);
    float r = v2.angle(v1);
    cross.unit();
    rot_ed_.rotate(r, cross);
    rot_ed_.unit();
  }

  // 緯度経度から向きを求める
  void rotate(const Vec2<float>& target, const bool reset = false)
  {
    Quat<float> qrx;
    qrx.rotate(target.x, Vec3<float>(1, 0, 0));
    Quat<float> qry;
    qry.rotate(target.y, Vec3<float>(0, 1, 0));
    rot_ed_ = qrx * qry;
    rot_ed_.unit();

    if (reset)
    {
      rot_st_ = rot_ed_;
      camera_.setRot(rot_st_);
    }
  }

  void distance(const float dist)
  {
    dist_ed_ = dist;
  }

  void randomDist()
  {
    picojson::array& array = params_["dist"].get<picojson::array>();
    dist_ed_ = dist_ed_org_ * array[rand() % array.size()].get<double>();
  }

  void randomRot(const bool reset)
  {
    float rx = ((rand() % 20) * (PI / 2.0)) / 20.0 - PI / 4.0;
    Quat<float> qrx;
    qrx.rotate(rx, Vec3<float>(1, 0, 0));

    float ry = PI - ((rand() % 20) * (PI * 2.0)) / 20.0;
    ry = PI;
    Quat<float> qry;
    qry.rotate(ry, Vec3<float>(0, 1, 0));

    rot_ed_ = qrx * qry;
    rot_ed_.unit();
    if (reset)
    {
      rot_st_ = rot_ed_;
      camera_.setRot(rot_st_);
    }
  }
  
  void step(const float delta_time)
  {
    if (delay_ > 0.0)
    {
      delay_ -= delta_time;
      if (delay_ > 0.0) return;
      rot_st_ = camera_.getRot();
      rot_st_.unit();
    }
    
    time_cur_ += delta_time;

    Easing easing;
    float dist;
    easing.ease(dist, time_cur_, dist_st_, dist_ed_, time_ed_, type_);
    camera_.setDist(dist);

    float t;
    easing.ease(t, time_cur_, 0.0, 1.0, time_ed_, type_);
    Quat<float> rot = slerp(rot_st_, rot_ed_, t);
    camera_.setRot(rot);
    
    bool active = (time_cur_ <= time_ed_);
    if (!active) this->finish();
  }

  void draw() {}

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_START:
    case MSG_TITLE_INTRO_SKIP:
      if (active_) 
      {
        finish();
      }
      break;

    case MSG_RESETCAMERA_ABORT:
      {
        active_ = false;
      }
      break;
    }
  }
};

}
