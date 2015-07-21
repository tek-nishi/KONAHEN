
#pragma once

//
// PNG_COLOR_TYPE_PALETTE   インデックスカラー
// PNG_COLOR_TYPE_RGB       RGB
// PNG_COLOR_TYPE_RGB_ALPHA RGBA
//

#include <cstring>
#include <vector>
#include "co_vec2.hpp"

#include <zlib.h>
#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "zlibd.lib")
#else
#pragma comment (lib, "zlib.lib")
#endif
#endif

#include <png.h>
#ifndef png_infopp_NULL 
#define png_infopp_NULL NULL
#endif

#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "libpngd.lib")
#else
#pragma comment (lib, "libpng.lib")
#endif
#endif


namespace ngs {

// TIPS:RAIIによる安全なpng_structリソース管理
class PngStruct
{
  png_struct *hdl_;
  png_info *info_;

public:
  PngStruct() :
    hdl_(),
    info_()
  {
    hdl_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    !hdl_ && DOUT << "Error:png_create_read_struct()" << std::endl;
    
    if (hdl_)
    {
      info_ = png_create_info_struct(hdl_);
      !info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
    }
  }

  ~PngStruct()
  {
    if (hdl_) png_destroy_read_struct(&hdl_, &info_, 0);
  }

  bool error() const { return !hdl_ || !info_; }
  png_struct *hdl() const { return hdl_; }
  png_info *info() const { return info_; }
};


void readFunc(png_struct *hdl, png_bytep buf, png_size_t size)
{
  png_bytep *p;

  p = static_cast<png_bytep *>(png_get_io_ptr(hdl));
  void *src = *p;
  std::memcpy(buf, src, size);
  *p += size;
}


class Png
{
  enum { PNG_COLOR_TYPE_NONE = -1 };

  std::vector<u_char> image_;
  
public:
  int type;
  Vec2<int> size;
    
  Png(const png_bytep ptr, const std::size_t length) :
    type(PNG_COLOR_TYPE_NONE)
  {
    DOUT << "Png()" << std::endl;

    if (!png_check_sig(ptr, length))
    {
      DOUT << "Error:png_check_sig()" << std::endl;
      return;
    }

    PngStruct png;                                  // FIXME:try~catchでできそう
    if (png.error()) return;
      
    png_bytep filepos = ptr;
    png_set_read_fn(png.hdl(), static_cast<png_voidp>(&filepos), static_cast<png_rw_ptr>(readFunc));

    png_read_info(png.hdl(), png.info());

    png_uint_32 width, height;
    int depth;
    png_get_IHDR(png.hdl(), png.info(), &width, &height, &depth, &type, 0, 0, 0);
    size.set(width, height);

    if (type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png.hdl());
    if (png_get_valid(png.hdl(), png.info(), PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png.hdl());
    if (depth == 16) png_set_strip_16(png.hdl());
    if (depth < 8) png_set_packing(png.hdl());

    png_read_update_info(png.hdl(), png.info());
    // どんなフォーマットもRGB8かRGBA8に収める

    size_t row = png_get_rowbytes(png.hdl(), png.info());
    type = ((row / width) == 3) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
    // 最終的なフォーマットを決定
    
    image_.resize(row * height);
    std::vector<png_bytep> row_pointers(height);
    for (size_t h = 0; h < height; ++h)
    {
      row_pointers[h] = &image_[h * row];
    }
    // あらかじめ読込先の領域を確保しておけばコピーは必要ない
    
    png_read_image(png.hdl(), &row_pointers[0]);
    png_read_end(png.hdl(), png.info());

    DOUT << "PNG:" << size.x << "x" << size.y << std::endl;
  }
  ~Png()
  {
    DOUT << "~Png()" << std::endl;
  }

  const u_char *image() const { return &image_[0]; }
};


#ifdef _DEBUG

//
// png書き出し
//
class PngWriteStruct
{
  png_struct *hdl_;
  png_info *info_;

public:
  PngWriteStruct() :
    hdl_(),
    info_()
  {
    hdl_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    !hdl_ && DOUT << "Error:png_create_write_struct()" << std::endl;
    
    if (hdl_)
    {
      info_ = png_create_info_struct(hdl_);
      !info_ && DOUT << "Error:png_create_info_struct()" << std::endl;
    }
  }

  ~PngWriteStruct()
  {
    if (hdl_) png_destroy_write_struct(&hdl_, &info_);
  }

  bool error() const { return !hdl_ || !info_; }
  png_struct *hdl() const { return hdl_; }
  png_info *info() const { return info_; }
};

// RGB8を書き出し
void WritePng(const std::string& file, const u_int width, const u_int height, u_char *image)
{
  // TIPS:OpenGLのフレームバッファは上下逆なので、
  //      書き出しテーブルを作るときにひっくり返す
  std::vector<png_bytep> row_pointers(height);
  for (size_t h = 0; h < height; ++h)
  {
    row_pointers[h] = &image[(height - 1 - h) * width * 4];
  }
  
  PngWriteStruct png;                               // FIXME:try~catchでできそう
  if (png.error()) return;

  FILE *fp = fopen(file.c_str(), "wb");
  if (!fp)
  {
    DOUT << "File create error:" << file << std::endl;
    return;
  }

  png_init_io(png.hdl(), fp);

  png_set_IHDR(png.hdl(), png.info(), width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png.hdl(), png.info());
  png_set_filler(png.hdl(), 0, PNG_FILLER_AFTER);

  png_write_image(png.hdl(), &row_pointers[0]);
  png_write_end(png.hdl(), 0);
  fclose(fp);
}

#endif

}
