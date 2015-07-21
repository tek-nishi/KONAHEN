
#pragma once

//
// Linux専用コード
//

#if defined (__linux__)

#include <iostream>
#include <string>
#include <streambuf>
#include <cstring>
#include <GL/glx.h>

namespace ngs {

class Os {
  const std::string path_;
  const std::string savePath_;

public:
  Os() :
    path_(""),
    savePath_("")
  {
    DOUT << "Os()" << std::endl;
  }
  ~Os() {
    DOUT << "~Os()" << std::endl;
  }

  // 実行環境のフルパスを返す
  const std::string& path() const { return path_; }
  const std::string& savePath() const { return savePath_; }

  bool isVsyncSwap() const
  {
    const char* ext = (const char *)glGetString(GL_EXTENSIONS);
    bool res = std::strstr(ext, "GLX_SGI_swap_control") ? true : false;

    DOUT << "GLX_SGI_swap_control:" << res << std::endl;

    return res;
  }

  bool toggleVsyncSwap(int sync) const
  {
    PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddress((const GLubyte *)"glXSwapIntervalSGI");
    return (glXSwapIntervalSGI) ? glXSwapIntervalSGI(sync) : false;
  }

};

}

#endif
