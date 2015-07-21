
#pragma once

//
// wavをリニアPCMで扱う
// ※扱えるのは量子化ビットが16bitのデータのみ
//

#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>


namespace ngs {

const char *getChunk(const char *ptr, const char *end, const char *chunk)
{
  while (std::strncmp(ptr, chunk, 4) && (ptr < end))
  {
    ptr += 4;
    u_int size = getNum(ptr, 4);
    ptr += 4;
    ptr += size;
  }

  if (ptr == end)
  {
    DOUT << "No '" << chunk << "' chunk" << std::endl;
  }

  return ptr;
}


class Pcm
{
  int ch_;
  int rate_;
  int size_;
  int samples_;
  std::vector<u_short> data_;

  bool read(const char *ptr)
  {
    if (std::strncmp(ptr, "RIFF", 4))
    {
      DOUT << "Not RIFF File." << std::endl;
      return false;
    }
    ptr += 4;

    u_int riff_size = getNum(ptr, 4);
    ptr += 4;
    const char *end = ptr + riff_size;

    if (std::strncmp(ptr, "WAVE", 4))
    {
      DOUT << "Not WAVE File." << std::endl;
      return false;
    }
    ptr += 4;

    ptr = getChunk(ptr, end, "fmt ");
    if (ptr == end) return false;

    {
      const char *data = ptr + 4;
      u_int size = getNum(data, 4);
      data += 4;
      DOUT << "'fmt' chunk:" << size << std::endl;

      u_int type = getNum(data, 2);                 // データタイプ
      data += 2;
      ch_ = getNum(data, 2);                        // チャンネル数
      data += 2;
      rate_ = getNum(data, 4);                      // サンプリングレート
      data += 4;
      // u_int bps = getNum(data, 4);                 // データ速度(Byte / sec)
      data += 4;
      // u_int block = getNum(data, 2);               // ブロックサイズ(byte/sample x チャンネル数)
      data += 2;
      u_int bitRate = getNum(data, 2);              // 量子化ビット数(8 16 24 …)

      if (type != 1 || bitRate != 16)
      {
        DOUT << "format error:" << type << " " << bitRate << std::endl;
        return false;
      }
    }
    
    ptr = getChunk(ptr, end, "data");
    if (ptr == end) return false;

    {
      const char *data = ptr + 4;
      size_ = getNum(data, 4);
      data += 4;
      DOUT << "'data' chunk:" << size_ << std::endl;

      samples_ = size_ / sizeof(u_short) / ch_;
      data_.resize(size_ / sizeof(u_short));
#ifdef __BIG_ENDIAN__
      const void *p = data;                         // reinterpret_cast？
      const u_short *src = static_cast<const u_short *>(p);
      for(std::vector<u_short>::iterator it = data_.begin(); it != data_.end(); ++it, ++src)
      {
        *it = (*src << 8) | (*src >> 8);
      }
#else
      std::memcpy(&data_[0], data, size_);
#endif
    }

    return true;
  }

public:
  explicit Pcm(const std::string& file)
  {
    std::ifstream fstr(file.c_str(), std::ios::binary);
    if(fstr.is_open())
    {
      std::size_t size = fstr.seekg(0, std::ios::end).tellg();
      std::vector<char> buf(size);
      fstr.seekg(0, std::ios::beg).read(&buf[0], size);
      this->read(&buf[0]);
    }
  }

  int channel() const { return ch_; }
  int rate() const { return rate_; }
  int size() const { return size_; }
  int samples() const { return samples_; }
  const u_short *data() const { return &data_[0]; }
};

}
