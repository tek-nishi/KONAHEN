
#pragma once

//
// アプリケーション本体
//

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <memory>
#include "co_vec2.hpp"
#include "co_touch.hpp"
#include "co_keyinp.hpp"
#include "co_timer_win.hpp"
#include "co_timer_osx.hpp"
#include "co_font.hpp"
#include "co_audio.hpp"
#include "co_easing.hpp"
#include "co_avarage.hpp"
#include "co_glex.hpp"
#include "nn_sound.hpp"
#include "nn_settings.hpp"
#include "nn_proc_base.hpp"
#include "nn_gameproc.hpp"
#include "nn_gamepre.hpp"
#include "nn_placeviewer.hpp"
#include "nn_easingtest.hpp"
#include "nn_earthviewer.hpp"
#include "nn_sandbox.hpp"


namespace ngs {

enum {
  APP_GAME_PRE,
  APP_GAMEMAIN,
  APP_PLACEVIEWER,
  APP_EASINGTEST,
  APP_EARTHVIEWER,
  APP_SANDBOX,
#if (TARGET_OS_IPHONE)
  FIRST_PROC = APP_GAMEMAIN
#elif defined (_DEBUG)
  FIRST_PROC = APP_GAMEMAIN
#else
  FIRST_PROC = APP_GAME_PRE
#endif
};


struct ProcInfo {
  const int       type;
  const Vec2<int> size;
  const float     scale;
  const bool      oblong;
  const bool      retina;

  Touch&    touch;
  Keyinp&   keyinp;
  Sound&    sound;
  Settings& settings;

  const std::string& path;
  const std::string& savePath;
  const std::string& lang;
};
// プロシージャを起動するのに与える情報が増えたので構造体にした


// FIXME:switch()による分岐を使わずに実装できないものか
std::tr1::shared_ptr<ProcBase> CreateProc(const ProcInfo& info)
{
  switch(info.type)
  {
#if !(TARGET_OS_IPHONE)
#ifdef _DEBUG
  case APP_PLACEVIEWER:
    {
      std::tr1::shared_ptr<ProcBase> proc(new PlaceViewer(info.size, info.scale,
                                                          info.touch, info.keyinp, info.path, info.lang));
      return proc;
    }
    break;

  case APP_EASINGTEST:
    {
      std::tr1::shared_ptr<ProcBase> proc(new EasingTest(info.size, info.scale,
                                                         info.touch, info.keyinp, info.path));
      return proc;
    }
    break;

  case APP_EARTHVIEWER:
    {
      std::tr1::shared_ptr<ProcBase> proc(new EarthViewer(info.size, info.scale, info.oblong,
                                                          info.touch, info.keyinp, info.path, info.lang));
      return proc;
    }
    break;

  case APP_SANDBOX:
    {
      std::tr1::shared_ptr<ProcBase> proc(new SandBox(info.size, info.scale,
                                                      info.touch, info.keyinp, info.path));
      return proc;
    }
    break;
#endif
  case APP_GAME_PRE:
    {
      std::tr1::shared_ptr<ProcBase> proc(new GamePre(info.size, info.path));
      return proc;
    }
#endif
  }

  std::tr1::shared_ptr<ProcBase> proc(new GameProc(info.size, info.scale, info.oblong, info.retina,
                                                   info.touch, info.keyinp, info.sound, info.settings,
                                                   info.path, info.savePath, info.lang));
  return proc;
}


class App
{
#define DEBUG_FONT  FONT_TEXTURE
  
  int width_, height_;
  int width_org_, height_org_;
  int cur_width_, cur_height_;
  float aspect_;
  float scale_;
  bool oblong_;
  bool retina_;

  const std::string path_;
  const std::string savePath_;
  const std::string lang_;
  
  Audio audio_;
  Sound sound_;
  Touch touch_;
  Keyinp keyinp_;
  Font font_;
  Settings settings_;

#ifdef NN_APP_DISP_FPS
  Timer t_root_;
  Avarage<double> dt1_;
  Avarage<double> dt2_;
#endif
  
  bool pause_;

  int app_type_;
  std::tr1::shared_ptr<ProcBase> proc_;

  void dispSize(const int w, const int h)
  {
    width_ = w;
    height_ = h;
    aspect_ = (float)w / (float)h;
  }

  void updateSettings()
  {
    picojson::object& obj = settings_.json().value().get<picojson::object>();

    obj["width"] = picojson::value((double)cur_width_);
    obj["height"] = picojson::value((double)cur_height_);
    obj["volume"] = picojson::value(audio_.getGain());
    obj["bgm_mute"] = picojson::value(audio_.mute(Audio::BGM));
    obj["se_mute"] = picojson::value(audio_.mute(Audio::SE));
  }
  
public:
  App(const int w, const int h, const float scale, const std::string& path, const std::string& savePath, const std::string& lang) :
    width_org_(w),
    height_org_(h),
    scale_(scale),
    oblong_(h > w),
    retina_(),
    path_(path),
    savePath_(savePath),
    lang_(lang),
    audio_(path_),
    sound_(audio_),
    font_(DEBUG_FONT, path + "devdata/VeraMono.ttf", 12),
    settings_(path_, savePath_),
#ifdef NN_APP_DISP_FPS
    dt1_(0, 60),
    dt2_(0, 60),
#endif
    pause_(),
    app_type_(FIRST_PROC)
  {
    DOUT << "App()" << std::endl;
    srand(time(0));

    picojson::object& obj = settings_.json().value().get<picojson::object>();
    cur_width_ = obj["width"].get<double>();
    cur_height_ = obj["height"].get<double>();
    audio_.setGain(obj["volume"].get<double>());
    audio_.mute(Audio::BGM, obj["bgm_mute"].get<bool>());
    audio_.mute(Audio::SE, obj["se_mute"].get<bool>());
    
    dispSize(w, h);

#ifdef TOUCH_RECORD
    std::string file(savePath + "touch.record");
    touch_.file(file);
#endif
  }
  
  ~App()
  {
    DOUT << "~App()" << std::endl;
    updateSettings();
    // TIPS:ソフトリセット時に設定を引き継ぎたいので
  }
  
  void resize(const int w, const int h)
  {
    cur_width_ = w;
    cur_height_ = h;

    float sx = (float)w / (float)width_;
    float sy = (float)h / (float)height_;

    this->dispSize(w, h);
    glViewport(0, 0, w, h);

    touch_.resize(Vec2<float>(w / scale_, h / scale_), Vec2<float>(0, 0), Vec2<float>(w, h));

    proc_->resize(w, h, sx, sy);
  }

  void y_bottom(const float y)
  {
    proc_->y_bottom(y);
  }
  
  void resizeFit(const int w, const int h)
  {
    cur_width_ = w;
    cur_height_ = h;

    glViewport(0, 0, w, h);
    
    touch_.resize(Vec2<float>(width_ / scale_, height_ / scale_), Vec2<float>(0, 0), Vec2<float>(w, h));

    proc_->resize(width_, height_);
  }
  
  void resizeKeepAspect(const int w, const int h)
  {
    cur_width_ = w;
    cur_height_ = h;

    bool a = ((float)w / (float)h) < aspect_;

    int width = a ? w : (float)h * aspect_ + 0.5;
    int height = a ? (float)w / aspect_ + 0.5 : h;

    int ofs_x = (w - width) / 2;
    int ofs_y = (h - height) / 2;
    glViewport(ofs_x, ofs_y, width, height);

    touch_.resize(Vec2<float>(width_ / scale_, height_ / scale_), Vec2<float>(ofs_x, ofs_y), Vec2<float>(w, h));

    proc_->resize(width, height);
  }

  void reset()
  {
    sound_.stopAll();
    proc_.reset();
    // 先に破棄してメモリの二重化を回避

    settings_.reload();
    // 意図的に書き出す前に戻す

    ProcInfo info = {
      app_type_,
      Vec2<int>(width_org_, height_org_),
      scale_,
      oblong_,
      retina_,
      touch_,
      keyinp_,
      sound_,
      settings_,
      path_,
      savePath_,
      lang_
    };
    proc_ = CreateProc(info);

    dispSize(width_org_, height_org_);
    // 内部的に初期サイズに戻す
#ifdef _DEBUG
    pause_ = false;
#endif
  }

  void next_proc()
  {
    proc_.reset();
    // 先に破棄してメモリの二重化を回避

    ProcInfo info = {
      app_type_,
      Vec2<int>(width_org_, height_org_),
      scale_,
      oblong_,
      retina_,
      touch_,
      keyinp_,
      sound_,
      settings_,
      path_,
      savePath_,
      lang_
    };
    proc_ = CreateProc(info);

    dispSize(width_org_, height_org_);
#ifdef _DEBUG
    pause_ = false;
#endif
  }

  void writeSettings()
  {
    DOUT << "App::writeSettings" << std::endl;
    updateSettings();
    settings_.write();
  }
  
  void update(const float delta_time)
  {
#ifdef NN_APP_DISP_FPS
    Timer timer;
#endif

    keyinp_.update();

    float scale = 1.0;
#ifdef _DEBUG
    if (keyinp_.get() == ASCII_ESC) pause_ = !pause_;
    if (keyinp_.press(INP_KEY_UP)) scale = 5.0;
    else
    if (keyinp_.press(INP_KEY_DOWN)) scale = 0.25;
#endif
    bool next_proc = false;
    if (!pause_)
    {
#ifdef TOUCH_RECORD
      touch_.step();
#endif
      next_proc = !proc_->step(delta_time * scale);
      if (next_proc)
      {
        if (app_type_ == APP_GAME_PRE) app_type_ = APP_GAMEMAIN;  
      }
      sound_.update(delta_time * scale);
    }
    proc_->draw();

#ifdef NN_APP_DISP_FPS
    double dt_1 = timer.get();
    dt_1 = (dt_1 > 0.0) ? 1.0 / dt_1 : 0.0;

    double dt_2 = t_root_.last();
    dt_2 = (dt_2 > 0.0) ? 1.0 / dt_2 : 0.0;

    double d1 = (dt1_ += dt_1);
    double d2 = (dt2_ += dt_2);

    std::stringstream sstr;
    sstr << "FPS:" << std::fixed << std::setw(5) << std::setprecision(2) << d2 << "(" << std::setw(7) << d1 << ")";

    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    font_.pos(Vec2<float>(((width_ / scale_) / -2.0), (height_ / scale_) / -2.0 + 12));
    font_.draw(sstr.str());
#endif

#ifdef _DEBUG
    bool do_reset = false;
    u_char key = keyinp_.get();
    if (key == INP_KEY_F1)
    {
      app_type_ = APP_GAMEMAIN;
      do_reset = true;
    }
    else if (key == INP_KEY_F2)
    {
      app_type_ = APP_PLACEVIEWER;
      do_reset = true;
    }
    else if (key == INP_KEY_F3)
    {
      app_type_ = APP_EASINGTEST;
      do_reset = true;
    }
    else if (key == INP_KEY_F4)
    {
      app_type_ = APP_EARTHVIEWER;
      do_reset = true;
    }
    else if (key == INP_KEY_F6)
    {
      app_type_ = APP_SANDBOX;
      do_reset = true;
    }
#ifdef TOUCH_RECORD
    else if (key == 'O')
    {
      touch_.record(!touch_.record());
      forceFrame(touch_.record());
      DOUT << "Record:" << touch_.record() << std::endl;
    }
    else if (key == 'P')
    {
      touch_.replay(!touch_.replay());
      forceFrame(touch_.replay());
      DOUT << "Replay:" << touch_.replay() << std::endl;
    }
#endif

    if (do_reset && !next_proc)
    {
      reset();
    }
#endif
    if (next_proc)
    {
      this->next_proc();
    }
  }
  
  bool pause() const { return pause_; }
  const std::string& path() const { return path_; }
  Touch& touch() { return touch_; }
  Keyinp& keyinp() { return keyinp_; }

  const int windowWidth() const { return cur_width_; }
  const int windowHeight() const { return cur_height_; }

  void retina(const bool val) { retina_ = val; }
  bool retina() const { return retina_; }
  
  ALCcontext *const audioContext() const { return audio_.context(); }

#ifdef _DEBUG
  // 処理時間を1/60固定
  void forceFrame(const bool force)
  {
    proc_->forceFrame(force);
  }
#endif
};

}
