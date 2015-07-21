
#pragma once

//
// ゲーム実行環境
//

#include <iostream>
#include <map>
#include "picojson.h"
#include "co_touch.hpp"
#include "co_keyinp.hpp"
#include "co_texmng.hpp"
#include "co_font.hpp"
#include "co_fntmng.hpp"
#include "co_json.hpp"
#include "co_task.hpp"
#include "nn_localize.hpp"
#include "nn_proc_base.hpp"
#include "nn_earth.hpp"
#include "nn_camera.hpp"
#include "nn_light.hpp"
#include "nn_misc.hpp"
#include "nn_place.hpp"
#include "nn_gameenv.hpp"
#include "nn_title.hpp"
#include "nn_gamemain.hpp"
#include "nn_game2dsetup.hpp"
#include "nn_gameworld.hpp"
#include "nn_toucheft.hpp"
#include "nn_gameintro.hpp"
#include "nn_control.hpp"
#include "nn_placedisp.hpp"
#include "nn_resetcamera.hpp"
#include "nn_fadelight.hpp"
#include "nn_stardust.hpp"
#include "nn_sound.hpp"
#include "nn_touchmenu.hpp"
#include "nn_settings.hpp"
#include "nn_gametimer_win.hpp"
#include "nn_gametimer_osx.hpp"
#include "nn_replay.hpp"


namespace ngs {

void SetupCamera(Camera& camera, picojson::object& param, float aspect)
{
  float angle = param["angle"].get<double>();
  camera.setAngle(angle * PI / 180.0);
  camera.setAspect(aspect);
  camera.setNearZ(param["near"].get<double>());
  camera.setFarZ(param["far"].get<double>());
  camera.setDist(param["dist"].get<double>());
  camera.pitch_max(Ang2Rad(param["pitch_max"].get<double>()));
}

void SetupLight(Light& light, picojson::object& param)
{
  {
    picojson::array& array = param["ambient"].get<picojson::array>();
    light.ambientCol(array[0].get<double>(), array[1].get<double>(), array[2].get<double>());
  }

  {
    picojson::array& array = param["diffuse"].get<picojson::array>();
    light.diffuseCol(array[0].get<double>(), array[1].get<double>(), array[2].get<double>());
  }

  {
    picojson::array& array = param["pos"].get<picojson::array>();
    light.pos(Vec3<float>(array[0].get<double>(), array[1].get<double>(), array[2].get<double>()));
  }

  {
    picojson::array& array = param["ofs"].get<picojson::array>();
    light.ofs(Vec3<float>(array[0].get<double>(), array[1].get<double>(), array[2].get<double>()));
  }
  
  if (param["cutoff"].is<double>())
  {
    light.cutoff(param["cutoff"].get<double>());
    light.exponent(param["exponent"].get<double>());
    light.spot(true);
  }
  
  if (param["handle"].is<double>())
  {
    light.handle(GL_LIGHT0 + param["handle"].get<double>());
  }
}


// 問題をすべて読み込む
void SetupPlaces(std::map<std::string, Place>& places, picojson::object& lists, const std::string& path)
{
#ifdef NO_READ_PLACES
  return;
#endif
  
  for (picojson::object::iterator it = lists.begin(); it != lists.end(); ++it)
  {
    std::string file = path + "devdata/place/" + it->second.get<picojson::object>()["file"].get<std::string>();
    Place place(file);
#ifdef _DEBUG
    if (places.find(it->first) != places.end())
    {
      DOUT << "multi place:" << it->first << std::endl;
      // 問題名が被っていたら警告
    }
#endif
    places.insert(std::map<std::string, Place>::value_type(it->first, place));
  }
}


class GameProc : public ProcBase
{
  Touch& touch_;
  Keyinp& keyinp_;
  Sound& sound_;
  Vec2<int> size_;
  float y_bottom_;

  Json params_;
  Json places_;
  Localize localize_;
  std::map<std::string, Place> place_data_;
  
  const float scale_;
  float aspect_;
  Camera camera_;
  Camera cockpit_;

  Earth earth_;
  std::vector<Light> lights_;

  FntMng fonts_;

  TexMng texMng_;

  GameEnv env_;
  Task task_;
  
  GameTimer timer_;
#ifdef _DEBUG
  bool forceFrame_;
#endif
  Replay *replay_;

  void setup()
  {
    DOUT << "GameProc::setup()" << std::endl;

    earth_.setRotSpeed(earth_.getRotSpeedOrig());
    task_.add<Title>(TASK_PRIO_2D, env_, *replay_);

    env_.demo  = false;
    env_.setup = false;
  }

  void cleanup()
  {
    DOUT << "GameProc::cleanup()" << std::endl;

    task_.sendMsgAll(MSG_GAME_CLEANUP);
    env_.settings->write();

    env_.game_mode = GAME_MODE_NORMAL;
    env_.cleanup   = false;
    env_.setup     = true;
  }
  
public:
  GameProc(const Vec2<int>& size, const float scale, const bool oblong, const bool retina, Touch& touch, Keyinp& keyinp, Sound& sound, Settings& settings, const std::string& path, const std::string& savePath, const std::string& lang) :
    touch_(touch),
    keyinp_(keyinp),
    sound_(sound),
    size_(size.x / scale, size.y / scale),
    y_bottom_(),
    params_(path + "devdata/params.json"),
    places_(path + params_.value().get<picojson::object>()["game"].get<picojson::object>()["places"].get<std::string>()),
    localize_(lang, path),
    scale_(scale),
    aspect_((float)size.x / (float)size.y),
    camera_(Camera::PERSPECTIVE),
    cockpit_(Camera::ORTHOGONAL),
    earth_(params_.value().get<picojson::object>()["earth"].get<picojson::object>(), path, camera_)
  {
    DOUT << "GameProc()" << std::endl;

    picojson::object& params = params_.value().get<picojson::object>();

    camera_.oblong(oblong);
    SetupCamera(camera_, params["camera"].get<picojson::object>(), aspect_);
    cockpit_.setSize(size.x / scale, size.y / scale);
    // カメラセットアップ

    {
      picojson::array& array = params["lights"].get<picojson::array>();
      for (picojson::array::iterator it = array.begin(); it != array.end(); ++it)
      {
        Light light;
        SetupLight(light, it->get<picojson::object>());
        lights_.push_back(light);
      }
      earth_.light(lights_);
      // 光源セットアップ
    }

    {
      picojson::array& array = params["texture"].get<picojson::array>();
      for(picojson::array::iterator it = array.begin(); it != array.end(); ++it)
      {
        texMng_.read(path + it->get<std::string>());
      }
      // texture先読み
    }

#if 0
    PreReadFont(fonts_, path, params);
    // FIXME:Font の先読みは効果がない(最初の描画で128文字ラスタライズされる)
#endif

    SetupPlaces(place_data_, places_.value().get<picojson::object>(), path);
    
#ifdef _DEBUG
    {
      picojson::object& pl = places_.value().get<picojson::object>();
      
      for (std::map<std::string, Place>::iterator it = place_data_.begin(); it != place_data_.end(); ++it)
      {
        picojson::object& p = pl[it->first].get<picojson::object>();
        localize_.get(p["name"].get<std::string>());
        if (p["answer"].is<std::string>())
        {
          localize_.get(p["answer"].get<std::string>());
        }
      }
      // 全問題、ローカライズ済みかチェック
    }
#endif

    env_.touch      = &touch_;
    env_.keyinp     = &keyinp_;
    env_.sound      = &sound_;
    env_.path       = &path;
    env_.savePath   = &savePath;
    env_.settings   = &settings;
    env_.size       = &size_;
    env_.y_bottom   = &y_bottom_;
    env_.scale      = scale_;
    env_.retina     = retina;
    env_.localize   = &localize_;
    env_.params     = &params_;
    env_.places     = &places_;
    env_.place_data = &place_data_;
    env_.fonts      = &fonts_;
    env_.task       = &task_;
    env_.texMng     = &texMng_;
    env_.camera     = &camera_;
    env_.cockpit    = &cockpit_;
    env_.earth      = &earth_;
    env_.earthLight = &lights_[0];
    env_.game_mode  = GAME_MODE_NORMAL;
    env_.hit        = false;
    env_.correct    = false;
    env_.demo       = false;
    env_.replay_num = 0;
    env_.setup      = true;
    env_.cleanup    = false;

    setupAchievement(env_);                         // 隠し要素のフラグは起動時に自動生成する

    {
      Task::ProcPtr t = env_.task->add<Replay>(TASK_PRIO_SYS, env_);
      replay_ = static_cast<Replay *>(t.get());
    }

    // 地球のテクスチャを決定する
    env_.earth_texture = settings.get<double>("texture");
    earth_.texture(env_.earth_texture);
  
    task_.add<Game2DSetup>(TASK_PRIO_2D_TOPPRIO, env_);
    task_.add<GameWorld>(TASK_PRIO_3D_TOPPRIO, env_);
    task_.add<StarDust>(TASK_PRIO_3D, env_);
    task_.add<TouchEft>(TASK_PRIO_3D, env_);
    {
      float margin = params["touch_margin"].get<double>();
      env_.touchMenu = task_.add<TouchMenu>(TASK_PRIO_SYS, touch_, margin);
    }
    task_.add<Control>(TASK_PRIO_SYS, env_);        // メニューより後で実行

    float delay = params["start_delay"].get<double>();
    {
      // カメラ設定
      camera_.setDist(params["start_dist"].get<double>());

      Task::ProcPtr t = task_.add<ResetCamera>(TASK_PRIO_3D, env_);
      ResetCamera *cam = static_cast<ResetCamera *>(t.get());
      cam->type(QUAD_OUT);
      cam->time(params["start_time"].get<double>());
      cam->delay(delay);
      if (env_.settings->get<bool>("played"))
      {
        cam->randomDist();
        cam->randomRot(true);
      }
      else
      {
        picojson::array& array = params["start_angle"].get<picojson::array>();
        Vec2<float> v(Ang2Rad(array[0].get<double>()), Ang2Rad(array[1].get<double>()));
        cam->rotate(v, true);
      }
    }

    {
      // 光源設定
      earth_.setLightScale(0.0);
      
      Task::ProcPtr t = task_.add<FadeLight>(TASK_PRIO_3D, env_);
      FadeLight *fl = static_cast<FadeLight *>(t.get());
      fl->type(QUAD_IN);
      fl->scale(1.0);
      fl->time(2.0);
      fl->delay(delay);
      // 初回だけの演出(dynamic_cast使わなくてよかった)
    }

#ifdef _DEBUG
    forceFrame_ = false;
#endif
  }
  ~GameProc()
  {
    DOUT << "~GameProc()" << std::endl;
  }

  void resize(const int w, const int h)
  {
    aspect_ = (float)w / (float)h;
    camera_.setAspect(aspect_);
  }
  void resize(const int w, const int h, const float sx, const float sy)
  {
    camera_.scale(sx, sy);
    cockpit_.setSize(w / scale_, h / scale_);
    size_.set(w / scale_, h / scale_);
  }

  void y_bottom(const float y) {
    y_bottom_ = y;
  }

  bool step(const float delta_time)
  {
    timer_.update();

    if (env_.setup) this->setup();
    if (env_.cleanup) this->cleanup();
    // FIXME:上の２つをなんとかしたい

    float dt = timer_.last();                       // FIXME:平均がいいのか即値がいいのか…
    if (dt < (delta_time * 0.5f)) dt = delta_time * 0.5f;
    else
    if (dt > (delta_time * 2.0f)) dt = delta_time * 2.0f;

#ifdef _DBUG
    if (forceFrame_) dt = delta_time;
#endif    
    task_.step(dt);

#ifdef _DEBUG
    u_char key = keyinp_.get();
    if (key == 't')
    {
      task_.dispTaskList();
    }
    else
    if (key == 'R')
    {
      env_.settings->reset();
      env_.settings->write();
      DOUT << "Reset settings." << std::endl;
      // セーブデータ初期化
      // FIXME:参照がおかしくなっているので、即時リセットする必要がある
    }
    else
    if (key == 'E')
    {
      localize_.reload("en.lang");
      DOUT << "Localize:en.lang" << std::endl;
      // 強制的に英語モード
    }
    else
    if (key == 'J')
    {
      localize_.reload("jp.lang");
      DOUT << "Localize:jp.lang" << std::endl;
      // 強制的に日本語モード
    }
    
#endif
    return true;
  }
  
  void draw()
  {
    task_.draw();
  }

#ifdef _DEBUG
  void forceFrame(const bool force) { forceFrame_ = force; }
#endif

};

}
