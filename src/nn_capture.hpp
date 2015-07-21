//
// 連続画面キャプチャー
//

#ifdef _DEBUG

#pragma once

#include "co_png.hpp"
#include "co_jpeg.hpp"
#include "nn_misc.hpp"

namespace ngs {

u_int number;

void StartCapture()
{
  number = 0;
}

void ExecCapture(const std::string& path, u_int width, u_int height)
{
  std::stringstream sstr;

  // sstr << path << "spapshot" << std::setw(5) << std::setfill('0') << number << ".png" << std::flush;
  // WriteFramebufferToPngFile(sstr.str(), width, height);
  sstr << path << "spapshot" << std::setw(5) << std::setfill('0') << number << ".jpg";
  WriteFramebufferToJpegFile(sstr.str(), width, height);

  ++number;
}

}

#endif
