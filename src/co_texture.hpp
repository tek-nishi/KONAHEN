
#pragma once

#include <string>
#include <fstream>
#include "co_vec2.hpp"
#include "co_png.hpp"
#include "co_pvrtc.hpp"
#include "co_filepath.hpp"
#include "co_misc.hpp"


namespace ngs {

void makeTexture(const Png& png, const int width, const int height, const bool mipmap)
{
  GLint type = (png.type == PNG_COLOR_TYPE_RGB) ? GL_RGB : GL_RGBA;
  DOUT << "Texture type:" << ((png.type == PNG_COLOR_TYPE_RGB) ? "RGB" : "RGBA") << std::endl;
  if (mipmap)
  {
    gluBuild2DMipmaps(GL_TEXTURE_2D, type, width, height, type, GL_UNSIGNED_BYTE, png.image());
  }
  else
  {
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, png.image());
  }
}

bool checkPvrtc(std::string& res, const std::string& path)
{
#if (TARGET_OS_IPHONE)
  std::string::size_type index = path.rfind(".png");
  if(index != std::string::npos)
  {
    res = path;
    const std::string ext = ".pvr";
    res.replace(index, ext.length(), ext);
    if (isFileExists(res)) return true;
  }
#endif
  return false;
}

void setupTextureParam(bool mipmap)
{
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST);
  // TODO:デバッグ用途で動的に変更したい

#if (TARGET_OS_IPHONE)
  // if (mipmap)
  // {
  //  GLfloat maxAniso;
  //  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
  //  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
  // }
  // FIXME:iPhone4で重い
#endif
}


class Texture
{
  Vec2<int> size_gl_;                               // OpenGLで管理するサイズ
  GLuint id_;
  Vec2<int> size_;
  Vec2<float> uv_;
  const std::string name_;
  bool mipmap_;

  void setupPng(std::ifstream& fstr, const bool mipmap)
  {
    std::size_t size = fstr.seekg(0, std::ios::end).tellg();
    std::vector<png_byte> buf(size);
    void *p = &buf[0];
    fstr.seekg(0, std::ios::beg).read(static_cast<char *>(p), size);
    fstr.close();

    Png png_obj(&buf[0], size);

    GLint pack, unpack;
    glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, id_);
    setupTextureParam(mipmap_);

    size_ = png_obj.size;
    size_gl_.set(int2pow(size_.x), int2pow(size_.y));
    uv_.set((float)size_.x / (float)size_gl_.x, (float)size_.y / (float)size_gl_.y);

    makeTexture(png_obj, size_gl_.x, size_gl_.y, mipmap);
        
    glPixelStorei(GL_PACK_ALIGNMENT, pack);
    glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
  }

#if (TARGET_OS_IPHONE)
  void setupPvrtc(std::ifstream& fstr)
  {
    std::size_t size = fstr.seekg(0, std::ios::end).tellg();
    std::vector<char> buf(size);
    fstr.seekg(0, std::ios::beg).read(&buf[0], size);
    fstr.close();

    Pvrtc obj(&buf[0]);
    size_gl_ = obj.size();
    size_ = size_gl_;
    uv_.set(1, 1);
    mipmap_ = obj.mipmap();

    glBindTexture(GL_TEXTURE_2D, id_);
    setupTextureParam(mipmap_);                     // TIPS: パラメータ設定はあらかじめ済ませておく
    obj.submit();
  }
#endif
  
public:
  Texture(const std::string& path, const bool mipmap = false) :
    name_(getFileName(path)),
    mipmap_()
  {
    DOUT << "Texture()" << std::endl;
    glGenTextures(1, &id_);

    std::string res;
    bool pvrtc = checkPvrtc(res, path);
    const std::string& p = pvrtc ? res : path;
    std::ifstream fstr(p.c_str(), std::ios::binary);
    if(fstr.is_open())
    {
#if (TARGET_OS_IPHONE)
      if (pvrtc)
      {
        this->setupPvrtc(fstr);
      }
      else
#endif
      {
        mipmap_ = mipmap;
        this->setupPng(fstr, mipmap);
      }
    }
  }
  ~Texture() {
    DOUT << "~Texture()" << std::endl;
    glDeleteTextures(1, &id_);
  }

  const Vec2<int>& size() const { return size_; }
  const Vec2<float>& uv() const { return uv_; }
  const std::string& name() const { return name_; }

  void bind() const
  {
    glBindTexture(GL_TEXTURE_2D, id_);
  }
};

}
