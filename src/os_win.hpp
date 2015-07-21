
#pragma once

//
// Windows専用コード
//

#if defined (_MSC_VER)

#include <iostream>
#include <string>
#include <streambuf>
#include <cstring>
#include <locale>


namespace ngs {

class DbgStreambuf : public std::streambuf
{
  std::vector<char> str_;
public:
  int_type overflow(int_type c = EOF)
  {
    str_.push_back(c);
    return c;
  }
  int sync()
  {
    str_.push_back('\0');                           // 念のため
    OutputDebugString(&str_[0]);
    str_.clear();
    return 0;
  }
};


class Os
{
  DbgStreambuf dbgStream_;
  std::streambuf *stream_;
  const std::string path_;
  const std::string savePath_;
  std::string lang_;

public:
  Os() :
    path_(""),
    savePath_(""),
    lang_("en.lang")
  {
    _set_error_mode(_OUT_TO_MSGBOX);
    stream_ = std::cout.rdbuf(&dbgStream_);
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    DOUT << "Os()" << std::endl;
    timeBeginPeriod(osTimerPeriod);

    std::locale lc("");
    DOUT << lc.name() << std::endl;
    if (lc.name() == std::string("Japanese_Japan.932"))
    {
      lang_ = "jp.lang";
    }
  }
  ~Os()
  {
    DOUT << "~Os()" << std::endl;
    timeEndPeriod(osTimerPeriod);
    std::cout.rdbuf(stream_);
  }

  // 実行環境のフルパスを返す
  const std::string& path() const { return path_; }
  const std::string& savePath() const { return savePath_; }
  const std::string& lang() const { return lang_; }

  // 画面更新がモニタと同期可能か調べる(glutの初期化後に呼び出す事)
  bool isVsyncSwap() const {
    const char* ext = (const char *)glGetString(GL_EXTENSIONS);
    bool res = std::strstr(ext, "WGL_EXT_swap_control") ? true : false;

    DOUT << "WGL_EXT_swap_control:" << res << std::endl;

    return res;
  }
    
  bool toggleVsyncSwap(int sync) const
  {
    typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );

    PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
    return (wglSwapIntervalEXT) ? wglSwapIntervalEXT(sync) : false;
  }
};

}

#endif
