
#pragma once

#include <string>
#include <vector>

extern "C" {
#include <jpeglib.h>
}
#if defined (_MSC_VER)
#ifdef _DEBUG
#pragma comment (lib, "jpegd.lib")
#else
#pragma comment (lib, "jpeg.lib")
#endif
#endif

namespace ngs {

// Jpeg書き出し
void WriteJpeg(const std::string& file, const u_int width, const u_int height, u_char *image)
{
  FILE *fp = fopen(file.c_str(), "wb");
  if (!fp)
  {
    DOUT << "File create error:" << file << std::endl;
    return;
  }
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;
 
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, fp);
 
  cinfo.image_width      = width;
  cinfo.image_height     = height;
  cinfo.input_components = 3;
  cinfo.in_color_space   = JCS_RGB;
  cinfo.dct_method       = JDCT_FLOAT;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 90, true);               // 圧縮率(0～100)
  jpeg_start_compress(&cinfo, true);

  // RGBA→RGBに泣く泣くコピー
  // ※iOSだとフレームバッファをRGBで取り出せない
  std::vector<JSAMPROW> array(height);
  std::vector<JSAMPLE> scanline(width * 3 * height);
  for (u_int i = 0; i < height; ++i)
  {
    JSAMPLE *src = &image[width * 4 * i];
    JSAMPLE *dst = &scanline[width * 3 * i];;
    for (u_int h = 0; h < width; ++h)
    {
      dst[0] = src[0];                              // R
      dst[1] = src[1];                              // G
      dst[2] = src[2];                              // B
      src += 4;
      dst += 3;
    }
    array[height - i - 1] = &scanline[width * 3 * i];
  }
  
  jpeg_write_scanlines(&cinfo, &array[0], height);
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  
  fclose(fp);
}

}
