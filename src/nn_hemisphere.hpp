
#pragma once

//
// 半球生成
//

#include <vector>

namespace ngs {

class Hemisphere {
  struct Vtx {
    GLfloat x, y, z;
  };
  struct Uv {
    GLfloat u, v;
  };
  struct Body {
    Vtx vtx;
    Vtx nor;
    Uv uv;
  };

  struct Face {
    GLushort v1, v2, v3;
  };

  std::vector<Body> body_;
  std::vector<Face> face_;

  GLuint vertices_;
  GLuint faces_;
  GLuint points_;

  bool texture_;

  GLuint vbo_[2];
  
public:
  Hemisphere(const int slices, const int stacks, const float radius, const bool ref = false) :
    vertices_((slices + 1) * (stacks + 1)),
    faces_(slices * stacks * 2),
    points_(faces_ * 3),
    texture_(true)
  {
    body_.reserve(256);
    face_.reserve(256);
    // あらかじめ予約しておくとpush_back()が軽くなる

    for(int j = 0; j <= stacks; ++j)
    {
      float ph = PI * (float)j / (float)stacks;
      float y = radius * cos(ph);
      float r = radius * sin(ph);
      for(int i = 0; i <= slices; ++i)
      {
        Body b;
        
        float th = PI * (float)i / (float)slices;
        float x = r * cos(th);
        float z = r * sin(th);
        b.vtx.x = x;
        b.vtx.y = y;
        b.vtx.z = z;

        float l = std::sqrt(x * x + y * y + z * z);
        b.nor.x = x / l;
        b.nor.y = y / l;
        b.nor.z = z / l;

        if (ref)
        {
          b.uv.u = b.nor.x * 0.5f + 0.5f;
          b.uv.v = b.nor.y * 0.5f + 0.5f;
          // 擬似環境マップUV
        }
        else
        {
          b.uv.u = (float)i / (float)slices;
          b.uv.v = (float)j / (float)stacks;
        }
        body_.push_back(b);
      }
    }

    for(int j = 0; j < stacks; ++j)
    {
      for(int i = 0; i < slices; ++i)
      {
        int count = (slices + 1) * j + i;

        Face face;

        face.v1 = count;
        face.v2 = count + slices + 2;
        face.v3 = count + 1;
        face_.push_back(face);
        
        face.v1 = count;
        face.v2 = count + slices + 1;
        face.v3 = count + slices + 2;
        face_.push_back(face);
        // GL_TRIANGLESで描画するのでfaceを２つづつ生成
      }
    }

    if (use_glex_vbo)
    {
      glGenBuffers(2, vbo_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(Body) * body_.size(), &body_[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * face_.size(), &face_[0], GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      // 割り当てを解除しておく
    }
  }
  
  ~Hemisphere()
  {
    if (use_glex_vbo)
    {
      glDeleteBuffers(2, vbo_);
    }
  }

  void texture(const bool texture) { texture_ = texture; }

  void draw()
  {
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    if (texture_)
    {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else
    {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (use_glex_vbo)
    {
      glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
      glVertexPointer(3, GL_FLOAT, sizeof(Body), 0);
      glNormalPointer(GL_FLOAT, sizeof(Body), (GLvoid *)(sizeof(Vtx)));
      if (texture_) glTexCoordPointer(2, GL_FLOAT, sizeof(Body), (GLvoid *)(sizeof(Vtx) + sizeof(Vtx)));
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
      glDrawElements(GL_TRIANGLES, points_, GL_UNSIGNED_SHORT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
      glVertexPointer(3, GL_FLOAT, sizeof(Body), &body_[0]);
      glNormalPointer(GL_FLOAT, sizeof(Body), (GLbyte *)(&body_[0]) + (sizeof(Vtx)));
      if (texture_) glTexCoordPointer(2, GL_FLOAT, sizeof(Body), (GLbyte *)(&body_[0]) + (sizeof(Vtx) + sizeof(Vtx)));

      glDrawElements(GL_TRIANGLES, points_, GL_UNSIGNED_SHORT, &face_[0]);
    }

    glPopClientAttrib();
  }
  
};

}

