
#pragma once

//
// ファイルの文字列を操作する関数群
//

#include <string>

namespace ngs {

std::string getFilePath(std::string path) {
  std::string::size_type end(path.rfind('/'));
  return (end != std::string::npos) ? path.substr(0, end) : std::string();
}

std::string getFileNameFull(std::string path) {
  return path.substr(path.rfind('/') + 1, path.length());
}

std::string getFileName(std::string path) {
  path = getFileNameFull(path);
  std::string name = path.substr(0, path.rfind('.'));
  return name;
}

std::string getFileExt(std::string path) {
  std::string::size_type pos(path.rfind('.'));
  return (pos != std::string::npos) ? path.substr(pos, path.length()) : std::string();
}

}
