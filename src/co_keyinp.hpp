
#pragma once

//
// キー入力
// C-a 1 C-b 2 …
// 

#include <iostream>
#include <vector>
#include <algorithm>

namespace ngs {

enum {
  ASCII_ENTER = 13,                                 // リターンキー
  ASCII_BS    = 8,                                  // BSキー
  ASCII_DEL   = 127,                                // DELキー
  ASCII_ESC   = 27,                                 // ESCキー
};

enum {
  CTRL_A = 1,
  CTRL_B,
  CTRL_C,
  CTRL_D,
  CTRL_E,
  CTRL_F,
  CTRL_G,
  CTRL_H,
  CTRL_I,
  CTRL_J,
  CTRL_K,
  CTRL_L,
  CTRL_M,
  CTRL_N,
  CTRL_O,
  CTRL_P,
  CTRL_Q,
  CTRL_R,
  CTRL_S,
  CTRL_T,
  CTRL_U,
  CTRL_V,
  CTRL_W,
  CTRL_X,
  CTRL_Y,
  CTRL_Z,
};

enum {
  INP_KEY_F1        = GLUT_KEY_F1 + 0x80,
  INP_KEY_F2        = GLUT_KEY_F2 + 0x80,
  INP_KEY_F3        = GLUT_KEY_F3 + 0x80,
  INP_KEY_F4        = GLUT_KEY_F4 + 0x80,
  INP_KEY_F5        = GLUT_KEY_F5 + 0x80,
  INP_KEY_F6        = GLUT_KEY_F6 + 0x80,
  INP_KEY_F7        = GLUT_KEY_F7 + 0x80,
  INP_KEY_F8        = GLUT_KEY_F8 + 0x80,
  INP_KEY_F9        = GLUT_KEY_F9 + 0x80,
  INP_KEY_F10       = GLUT_KEY_F10 + 0x80,
  INP_KEY_F11       = GLUT_KEY_F11 + 0x80,
  INP_KEY_F12       = GLUT_KEY_F12 + 0x80,
  INP_KEY_UP        = GLUT_KEY_UP + 0x80,
  INP_KEY_DOWN      = GLUT_KEY_DOWN + 0x80,
  INP_KEY_LEFT      = GLUT_KEY_LEFT + 0x80,
  INP_KEY_RIGHT     = GLUT_KEY_RIGHT + 0x80,
  INP_KEY_PAGE_UP   = GLUT_KEY_PAGE_UP + 0x80,
  INP_KEY_PAGE_DOWN = GLUT_KEY_PAGE_DOWN + 0x80,
  INP_KEY_HOME      = GLUT_KEY_HOME + 0x80,
  INP_KEY_END       = GLUT_KEY_END + 0x80,
  INP_KEY_INSERT    = GLUT_KEY_INSERT + 0x80,
};

const std::size_t KEY_INPUT_KINDNUM = 256;

class Keyinp {
  struct keyrec {
    u_char key;
    bool push;
  };

  int que_page_;
  std::vector<keyrec> inp_que_[2];
  u_char last_key_[2];
  std::vector<char> key_press_;

public:
  Keyinp() :
    que_page_(),
    key_press_(KEY_INPUT_KINDNUM)
  {
    DOUT << "Keyinp()" << std::endl;
    last_key_[0] = '\0';
    last_key_[1] = '\0';
    // FIXME:コンストラクタで書きたい
  }
  ~Keyinp() {
    DOUT << "~Keyinp()" << std::endl;
  }

  void pushCallback(const u_char key, const int x, const int y) {
    // キーボードから押せないであろうキーの入力は無視する
    if(key < 0x80)
    {
      keyrec rec = { key, true };
      inp_que_[que_page_].push_back(rec);
      last_key_[que_page_] = key;
      key_press_[key] = 1;
    }
  }

  void pullCallback(const u_char key, const int x, const int y) {
    // キーボードから押せないであろうキーの入力は無視する
    if(key < 0x80)
    {
      keyrec rec = { key, false };
      inp_que_[que_page_].push_back(rec);
      key_press_[key] = 0;
    }
  }

  void exPushCallback(const int key, const int x, const int y) {
    keyrec rec = { static_cast<u_char>(key + 0x80), true };
    inp_que_[que_page_].push_back(rec);
    last_key_[que_page_] = key + 0x80;
    key_press_[key + 0x80] = 1;
  }

  void exPullCallback(const int key, const int x, const int y) {
    keyrec rec = { static_cast<u_char>(key + 0x80), false };
    inp_que_[que_page_].push_back(rec);
    key_press_[key + 0x80] = 0;
  }

  void repeat(const bool aRep) {
    glutIgnoreKeyRepeat(aRep ? 0 : 1);
  }

  void update() {
    que_page_ ^= 1;
    inp_que_[que_page_].clear();
    last_key_[que_page_] = '\0';
  }

  u_char get() const { return last_key_[que_page_ ^ 1]; }
  bool press(u_char key) const { return key_press_[key] ? true : false; }

  struct findPush {
    int key_;

    findPush(const int key) { key_ = key;}
    bool operator()(const keyrec& obj) const {
      return obj.push && (obj.key == key_);
    }
  };

  struct findPull {
    int key_;

    findPull(const int key) { key_ = key;}
    bool operator()(const keyrec& obj) const {
      return !obj.push && (obj.key == key_);
    }
  };

  bool push(const u_char key) {
    std::vector<keyrec>& p = inp_que_[que_page_ ^ 1];
    return std::find_if(p.begin(), p.end(), findPush(key)) != p.end();
  }
  bool pull(const u_char key) {
    std::vector<keyrec>& p = inp_que_[que_page_ ^ 1];
    return std::find_if(p.begin(), p.end(), findPull(key)) != p.end();
  }
};

}
