
#pragma once

//
// OSX専用コード
//

#if defined (__APPLE__)

#include <OpenGL/OpenGL.h>
#include <string>
#include <iostream>
#include <streambuf>
#include <CoreFoundation/CoreFoundation.h>


namespace ngs {

// 環境によっては起動パスからデータを読み込めないので、自力でパスを求める
std::string getCurrentPath()
{
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
  std::string path("");

  char str[PATH_MAX];
  if(!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)str, PATH_MAX))
  {
    DOUT << "error CFURLGetFileSystemRepresentation()" << std::endl;
  }
  else
  {
    path = std::string(str) + std::string("/");
  }
  CFRelease(resourcesURL);

  return path;
}


class Os {
  const std::string path_;
  const std::string savePath_;
  std::string lang_;

public:
  Os() :
    path_(getCurrentPath()),
    savePath_(path_)
  {
    DOUT << "Os()" << std::endl;

    CFStringRef langFile = CFCopyLocalizedString(CFSTR("langFile"), 0);
    char str[PATH_MAX];
    bool result = CFStringGetCString(langFile, str, PATH_MAX - 1, kCFStringEncodingUTF8);
    assert(result);
    lang_ = std::string(str);
    // TIPS:ローカライズ設定から読み込むファイルを決定する
  }
  ~Os()
  {
    DOUT << "~Os()" << std::endl;
  }

  const std::string& path() const { return path_; }
  const std::string& savePath() const { return savePath_; }
  const std::string& lang() const { return lang_; }

  bool isVsyncSwap() const { return true; }
  // Appleの資料を信じるならば、どのMacでも対応している

  bool toggleVsyncSwap(int sync) const
  {
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &sync);
    return true;
  }
};

}

#endif
