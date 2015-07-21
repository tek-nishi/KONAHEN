
#pragma once

//
// 「こなへん！」表示
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

class Konahen : public TaskProc
{
  GameEnv& env_;
  picojson::object& params_;
  bool active_;

  const TexMng::tex_ptr texture_;
  const Earth& earth_;

  Vec3<float> hit_pos_;

  const int just_;
  const bool miss_;
  const int div_;

  struct Vtx {
    GLfloat x, y;
  };
  struct Uv {
    GLfloat u, v;
  };
  struct Body {
    Vtx vtx;
    Uv uv;
  };
  std::vector<Body> body_;

  GLuint vbo_;

  float r_, rot_;
  float combo_;
  float size_;
  EaseArray<Vec2<float> > ease_;
  Vec2<float> res_;
  float rot_speed_;
  GrpCol<float> col_;
  
  float time_;
  bool step_;
  bool draw_;
  
public:
  Konahen(GameEnv& env, const std::string& name) :
    env_(env),
    params_(env.params->value().get<picojson::object>()[name].get<picojson::object>()),
    active_(true),
    texture_(env.texMng->read(*env.path + params_["texture"].get<std::string>())),
    earth_(*env.earth),
    just_(params_["just"].get<double>()),
    miss_(params_["miss"].get<bool>()),
    div_(params_["div"].get<double>()),
    r_(),
    rot_(params_["rotate"].get<double>()),
    combo_(params_["combo"].get<double>()),
    size_(params_["size"].get<double>()),
    rot_speed_(),
    time_(),
    step_(),
    draw_()
  {
    DOUT << "Konahen()" << std::endl;

    EasingArayVec2(CUBIC_INOUT, params_["easing"].get<picojson::array>(), ease_);

    const Vec2<int>& size = texture_->size();
    const Vec2<float>& uv = texture_->uv();
    const float loop = params_["loop"].get<double>();
    float btm_v = params_["v"].get<double>() * uv.y / size.y;
    float height = params_["height"].get<double>();

    {
      picojson::array& array = params_["col"].get<picojson::array>();
      col_.set(array[0].get<double>(), array[1].get<double>(), array[2].get<double>(), 1);
    }

    texture_->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    body_.reserve(64);
    for(int i = 0; i <= div_; i += 1)
    {
      float r = (PI * -2.0f * (float)i) / div_;
      float sin_r = sin(r);
      float cos_r = cos(r);

      Body b;
      b.vtx.x = sin_r;
      b.vtx.y = cos_r;
      b.uv.u = loop * (float)i / (float)div_;
      b.uv.v = 0;
      body_.push_back(b);
      
      b.vtx.x = height * sin_r;
      b.vtx.y = height * cos_r;
      b.uv.u = loop * (float)i / (float)div_;
      b.uv.v = btm_v;
      body_.push_back(b);
    }

    if (use_glex_vbo)
    {
      glGenBuffers(1, &vbo_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Body) * body_.size(), &body_[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      // 割り当てを解除しておく
    }
  }
  
  ~Konahen() {
    DOUT << "~Konahen()" << std::endl;
    if (use_glex_vbo)
    {
      glDeleteBuffers(1, &vbo_);
    }
  }

  bool active() const { return active_; }
  
  void step(const float delta_time)
  {
    if (!step_) return;
    
    r_ += rot_speed_ * delta_time;
    time_ += delta_time;
    step_ = ease_.ease(res_, time_);
    draw_ = step_;
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

    glRotatef(Rad2Ang(r), cross.x, cross.y, cross.z);
    glTranslatef(0, 0, earth_.getRadius());
    glScalef(size_ * res_.x, size_ * res_.x, 1);
    glRotatef(r_, 0, 0, 1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    texture_->bind();
    glColor4f(col_.r, col_.g, col_.b, res_.y);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if (use_glex_vbo)
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glVertexPointer(2, GL_FLOAT, sizeof(Body), 0);
      glTexCoordPointer(2, GL_FLOAT, sizeof(Body), (GLvoid *)(sizeof(Vtx)));
    
      glDrawArrays(GL_TRIANGLE_STRIP, 0, body_.size());

      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
      glVertexPointer(2, GL_FLOAT, sizeof(Body), &body_[0]);
      glTexCoordPointer(2, GL_FLOAT, sizeof(Body), (GLbyte *)(&body_[0]) + (sizeof(Vtx)));
      glDrawArrays(GL_TRIANGLE_STRIP, 0, body_.size());
    }
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
  }

  void msg(const int msg)
  {
    switch (msg)
    {
    case MSG_GAME_TOUCH_IN:
      if ((env_.just >= just_) && !miss_)
      {
        r_ = 0;
        time_ = 0;
        float just = (env_.just < 10) ? env_.just : 10.0;
        rot_speed_ = rot_ + rot_ * combo_ * just;
        step_ = true;
        hit_pos_ = env_.hit_pos;
      }
      break;

    case MSG_GAME_TOUCH_ANS:
      if (miss_)
      {
        r_ = 0;
        time_ = 0;
        rot_speed_ = rot_;
        step_ = true;
        hit_pos_ = env_.hit_pos;
      }
      break;
      
    case MSG_GAME_CLEANUP:
      {
        active_ = false;
      }
      break;
    }
  }
};

}
