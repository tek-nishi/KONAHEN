
#pragma once

//
// ヒント機能
//

#include <iostream>
#include <string>
#include "co_vec2.hpp"
#include "co_vec3.hpp"
#include "co_quat.hpp"
#include "co_matrix.hpp"
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class HintCircle : public TaskProc
{
  GameEnv& env_;
  picojson::object& params_;
  bool active_;

  const Camera& camera_;
  const Earth& earth_;
  const float radius_;

  const float scale_;
  const int div_;
  float width_;

  std::vector<GLfloat> vtx_;
  GLuint vbo_;

  Vec3<float> hit_pos_;
  Vec3<float> place_pos_;
  int hint_count_;

  const float rot_speed_;
  float rot_;
  
  float time_;
  float time_ed_;
  float angle_;
  float angle_st_;
  float angle_ed_;

  float alpha_time_;
  float alpha_time_ed_;
  float alpha_;
  float alpha_st_;
  float alpha_ed_;

  float fade_time_;
  const float fade_time_ed_;
  float fade_alpha_;
  float fade_alpha_st_;
  float fade_alpha_ed_;

  bool pause_;
  bool draw_stop_;
  bool draw_;
  
public:
  HintCircle(GameEnv& env) :
    env_(env),
    params_(env.params->value().get<picojson::object>()["hintcircle"].get<picojson::object>()),
    active_(true),
    camera_(*env.camera),
    earth_(*env.earth),
    radius_(earth_.getRadius()),
    scale_(params_["scale"].get<double>()),
    div_(params_["div"].get<double>()),
    width_(params_["width"].get<double>()),
    hint_count_(),
    rot_speed_(params_["rot_speed"].get<double>()),
    rot_(),
    time_ed_(params_["time_ed"].get<double>()),
    time_(),
    angle_(),
    angle_st_(),
    angle_ed_(),
    alpha_time_(),
    alpha_time_ed_(),
    alpha_(1.0f),
    alpha_st_(alpha_),
    alpha_ed_(alpha_),
    fade_time_(100),
    fade_time_ed_(0.3),
    fade_alpha_(1),
    pause_(),
    draw_stop_(),
    draw_()
  {
    DOUT << "HintCircle()" << std::endl;

    if (env_.retina) width_ *= 2.0f;

    vtx_.reserve(div_ * 2);
    for(int i = 0; i < div_; ++i)
    {
      float r = (PI * 2.0f * (float)i) / div_;
      vtx_.push_back(sin(r));
      vtx_.push_back(cos(r));

      r += (PI * 2.0f * 0.5f) / div_;
      vtx_.push_back(sin(r));
      vtx_.push_back(cos(r));
    }

    if (use_glex_vbo)
    {
      glGenBuffers(1, &vbo_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vtx_.size(), &vtx_[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      // 割り当てを解除しておく
    }
  }
  
  ~HintCircle() {
    DOUT << "~HintCircle()" << std::endl;

    if (use_glex_vbo)
    {
      glDeleteBuffers(1, &vbo_);
    }
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (!draw_) return;

    rot_ += rot_speed_ * delta_time;

    if (time_ < time_ed_)
    {
      time_ += delta_time;

      Easing easing;
      easing.ease(angle_, time_, angle_st_, angle_ed_, time_ed_, CUBIC_OUT);
      // ヒントサークルの大きさをイージング
    }

    if (alpha_time_ < alpha_time_ed_)
    {
      alpha_time_ += delta_time;

      Easing easing;
      easing.ease(alpha_, alpha_time_, alpha_st_, alpha_ed_, alpha_time_ed_, CUBIC_OUT);
      // ヒントサークルのアルファをイージング

      if (draw_stop_ && (alpha_time_ >= alpha_time_ed_))
      {
        draw_ = false;
      }
    }

    if (fade_time_ < fade_time_ed_)
    {
      fade_time_ += delta_time;

      Easing easing;
      easing.ease(fade_alpha_, fade_time_, fade_alpha_st_, fade_alpha_ed_, fade_time_ed_);
      // ポーズ時は表示を消す
    }
  }

  void draw()
  {
    if (!draw_) return;

    glPushMatrix();
    glLoadMatrixd(&(earth_.mtx()[0]));

    Vec3<float> v1(0, 0, 1);
    Vec3<float> v2(hit_pos_.x, hit_pos_.y, hit_pos_.z);
    v2.unit();
    Vec3<float> cross = v1.cross(v2);
    float r = v1.angle(v2);
    float ry = cos(angle_) * radius_;
    float rx = sin(angle_) * radius_ * scale_;
    glRotatef(Rad2Ang(r), cross.x, cross.y, cross.z);
    glTranslatef(0, 0, ry);
    glScalef(rx, rx, 1);
    glRotatef(Rad2Ang(rot_), 0, 0, 1);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    GrpSetBlend(GRP_BLEND_ADD);
    glLineWidth(width_);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);

    static const GLfloat f_col[] = { 0, 0, 0, 1 };
    glFogfv(GL_FOG_COLOR, f_col);

    float d = camera_.getDist();
    float rot = asin((radius_) / d);
    float c_r = cos(rot);
    float x = d * c_r;
    float dist = x * c_r;

    float z_dist = 300 + 50 - 50 * (d / camera_.getFarZ());
    glFogf(GL_FOG_START, dist);
    glFogf(GL_FOG_END, dist + z_dist);

    glEnableClientState(GL_VERTEX_ARRAY);
    glColor4f(0, 0, 1, alpha_ * fade_alpha_);
    if (use_glex_vbo)
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glVertexPointer(2, GL_FLOAT, 0, 0);
      glDrawArrays(GL_LINES, 0, vtx_.size() / 2);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
      glVertexPointer(2, GL_FLOAT, 0, &vtx_[0]);
      glDrawArrays(GL_LINES, 0, vtx_.size() / 2);
    }

    glDisable(GL_FOG);
    glDisable(GL_LINE_SMOOTH);

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

    case MSG_GAME_PLACEDISP_START:
      {
        hint_count_ = params_["hint_count"].get<double>();
      }
      break;
      
    case MSG_GAME_TOUCH_OUT:
      if ((hint_count_ > 0) && !(--hint_count_ % 3))
      {
        time_ = 0.0f;
        hit_pos_ = env_.hit_pos;
        place_pos_ = env_.place_center[0];
        angle_ed_ = hit_pos_.angle(place_pos_);
        angle_st_ = 0.0f;
        angle_ = angle_st_;

        float time_ed = params_["time_ed"].get<double>();
        time_ed_ = time_ed * (angle_ed_ / PI);

        alpha_time_ = 0.0f;
        alpha_time_ed_ = time_ed_;
        alpha_st_ = 0.5f;
        alpha_ed_ = 1.0f;
        alpha_ = alpha_st_;

        draw_ = true;
        draw_stop_ = false;
        env_.sound->play("hint");
        env_.task->sendMsgAll(MSG_GAME_HINT);
      }
      break;

    case MSG_GAME_TOUCH_IN:
    case MSG_GAME_TOUCH_ANS:
    case MSG_GAME_TOUCH_DISP:
      if (draw_)
      {
        draw_stop_ = true;
        
        alpha_time_ = 0.0f;
        alpha_time_ed_ = 0.4f;
        alpha_st_ = alpha_;
        alpha_ed_ = 0.0f;
      }
      break;

    case MSG_GAME_PAUSE:
      {
        pause_ = !pause_;

        fade_time_ = 0.0;
        fade_alpha_st_ = fade_alpha_;
        fade_alpha_ed_ = pause_ ? 0.0 : 1.0;
      }
      break;
    }
  }
};

}
