
#pragma once

//
// FTBitmapFont
// FTBufferFont
// FTExtrudeFont
// FTOutlineFont
// FTPixmapFont
// FTPolygonFont
// FTTextureFont
// TODO:'「''」'などで、表示サイズが間違っている
// 

#define FTGL_LIBRARY_STATIC
#if (TARGET_OS_IPHONE)
#include <FTGL/ftgles.h>
#else
#include <FTGL/ftgl.h>
#endif
#include <string>
#include "co_vec2.hpp"
#include "co_graph.hpp"


#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "freetyped.lib")
#pragma comment (lib, "ftgld.lib")
#else
#pragma comment (lib, "freetype.lib")
#pragma comment (lib, "ftgl.lib")
#endif
#endif

namespace ngs {

enum {
  FONT_BITMAP,
  FONT_BUFFER,                                      // 一行ごとテクスチャを作成
#if !(TARGET_OS_IPHONE)
  FONT_EXTRUDE,
#endif
  FONT_OUTLINE,                                     // アウトラインポリゴンを生成
  FONT_PIXMAP,
  FONT_POLYGON,                                     // ポリゴン生成
  FONT_TEXTURE,                                     // 一文字ずつテクスチャを作成
};

class Font {
  FTFont *font_;

  int blend_;
  int height_;
  Vec2<float> pos_;
  Vec2<float> center_;
  Vec2<float> scale_;
  GrpCol<float> col_;
  float rotate_;
  
public:
  Font(const int type, const std::string& name, const int height) :
    font_(),
    blend_(GRP_BLEND_NORMAL),
    height_(height),
    scale_(1, 1),
    col_(1, 1, 1, 1),
    rotate_()
  {
    DOUT << "Font()" << std::endl;
    
    switch(type)
    {
    case FONT_BITMAP:
      DOUT << "create: FONT_BITMAP" << std::endl;
      font_ = new FTBitmapFont(name.c_str());
      break;

    case FONT_BUFFER:
      DOUT << "create: FONT_BUFFER" << std::endl;
      font_ = new FTBufferFont(name.c_str());
      break;

#if !(TARGET_OS_IPHONE)
    case FONT_EXTRUDE:
      DOUT << "create: FONT_EXTRUDE" << std::endl;
      font_ = new FTExtrudeFont(name.c_str());
      break;
#endif

    case FONT_OUTLINE:
      DOUT << "create: FONT_OUTLINE" << std::endl;
      font_ = new FTOutlineFont(name.c_str());
      break;

    case FONT_PIXMAP:
      DOUT << "create: FONT_PIXMAP" << std::endl;
      font_ = new FTPixmapFont(name.c_str());
      break;

    case FONT_POLYGON:
      DOUT << "create: FONT_POLYGON" << std::endl;
      font_ = new FTPolygonFont(name.c_str());
      break;

    case FONT_TEXTURE:
      DOUT << "create: FONT_TEXTURE" << std::endl;
      font_ = new FTTextureFont(name.c_str());
      break;
    }
    font_->FaceSize(height);
    font_->UseDisplayList(false);
  }
  ~Font()
  {
    DOUT << "~Font()" << std::endl;
    delete font_;
  }

  void blend(const int mode) { blend_ = mode; }
  void pos(const Vec2<float>& val) { pos_ = val; }
  void pos(const float x, const float y) { pos_.x = x; pos_.y = y; }
  void center(const Vec2<float>& val)
  {
    center_.x = (int)val.x;
    center_.y = (int)val.y;
    // TIPS:なるべく小数点以下の座標に表示されないようにする
  }
  void center(const float x, const float y)
  {
    center_.x = (int)x;
    center_.y = (int)y;
  }
  void scale(const Vec2<float>& val) { scale_ = val; }
  void scale(const float x, const float y) { scale_.x = x; scale_.y = y; }
  void col(GrpCol<float>& val) {col_ = val; }
  void col(float r, float g, float b, float a = 1.0f)
  {
    col_.r = r;
    col_.g = g;
    col_.b = b;
    col_.a = a;
  }
  void rotate(const float rot) {rotate_ = rot; }
  void height(const int value) {
    height_ = value;
    font_->FaceSize(value);
  }
  int height() { return height_; }

  template <typename T>
  const Vec2<float> size(const T& str)
  {
    float llx, lly, llz;
    float urx, ury, urz;
    font_->BBox(str.c_str(), llx, lly, llz, urx, ury, urz);
    return Vec2<float>(urx - llx, ury - lly);
  }
  
  template <typename T>
  void draw(const T& str)
  {
    glPushMatrix();
    glTranslatef(pos_.x, pos_.y, 0);
    glRotatef(Rad2Ang(rotate_), 0, 0, 1);
    glScalef(scale_.x, scale_.y, 1.0);
    glTranslatef(-center_.x, -center_.y, 0);
    glScalef(1.0, -1.0, 1.0);
    GrpSetBlend(blend_);
    glColor4f(col_.r, col_.g, col_.b, col_.a);
    
    font_->Render(str.c_str());
    glPopMatrix();
  }

  template <typename T>
  void draw(const T& str, Vec2<float>pos)
  {
    glPushMatrix();
    FTPoint position(pos.x, pos.y);
    font_->Render(str.c_str(), -1, position, FTPoint());
    glPopMatrix();
  }
};

}
