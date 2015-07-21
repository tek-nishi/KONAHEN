
#pragma once

//
// 背景の星を表示
//

#include "co_vec3.hpp"
#include "co_matrix.hpp"
#include "nn_camera.hpp"


namespace ngs {

class Space {
  picojson::object& params_;
  const Camera& camera_;
  const u_int num_;

  struct Vtx {
    GLfloat x, y, z;
  };
  struct Color {
    GLfloat r, g, b, a;
  };
  struct Star {
    Vtx vtx;
    Color col;
  };
  std::vector<Star> star_;
  // OpenGLのストライドに対応する為、少々回りくどい定義になっている
  
  GLuint vbo_;
  
public:
  Space(picojson::object& params, const Camera& camera) :
    params_(params),
    camera_(camera),
    num_(params_["star_num"].get<double>())
  {
    DOUT << "Space()" << std::endl;

    const picojson::array& array = params_["star_col"].get<picojson::array>();
    std::vector<GrpCol<float> > colTbl;
    for (picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
    {
      const picojson::array& col = it->get<picojson::array>();
      GrpCol<float> c(col[0].get<double>(), col[1].get<double>(), col[2].get<double>(), 1);
      colTbl.push_back(c);
      // 色データを読み込んでおく
    }

    const float radius = params_["star_radius"].get<double>();
    star_.reserve(64);
    // あらかじめ適当な値を予約しておけば、push_backも軽い
    for (u_int i = 0; i < num_; ++i)
    {
      float ry = (rand() % 180) / 180.0f * PI * 2.0f;
      float rz = (rand() % 180) / 180.0f * PI * 2.0f;
      
      float x = sin(ry);
      float z = cos(ry);
      float y = sin(rz) * x;
      x = cos(rz) * x;
      // xz平面で回転→xy平面で回転

      const GrpCol<float>& col = colTbl[rand() % colTbl.size()];
      Star star = {
        { x * radius, y * radius, z * radius },
        { col.r, col.g, col.b, col.a }
      };
      star_.push_back(star);
    }

    if (use_glex_vbo)
    {
      glGenBuffers(1, &vbo_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Star) * star_.size(), &star_[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);               // 割り当てを解除しておく
    }
    
  }

  ~Space()
  {
    DOUT << "~Space()" << std::endl;

    if (use_glex_vbo)
    {
      glDeleteBuffers(1, &vbo_);
    }
  }
  
  void step(const float delta_time)
  {
  }
  
  void draw()
  {
    glPushMatrix();

    Matrix mtx;
    mtx.rotate(camera_.getRot());
    glLoadMatrixd(mtx.value());   

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH);

    glPointSize(1.5);
    
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    if(use_glex_vbo)
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_);
      glVertexPointer(3, GL_FLOAT, sizeof(Star), 0);
      glColorPointer(4, GL_FLOAT, sizeof(Star), (GLvoid *)(sizeof(Vtx)));

      glDrawArrays(GL_POINTS, 0, num_);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
      glVertexPointer(3, GL_FLOAT, sizeof(Star), &star_[0]);
      glColorPointer(4, GL_FLOAT, sizeof(Star), (GLbyte *)(&star_[0]) + (sizeof(Vtx)));
      glDrawArrays(GL_POINTS, 0, num_);
    }

    glPopClientAttrib();
    glPopMatrix();
  }
  
};

}
