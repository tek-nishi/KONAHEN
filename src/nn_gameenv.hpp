
#pragma once

//
// ゲーム環境
//

#include <string>
#include "co_vec2.hpp"
#include "co_touch.hpp"
#include "co_keyinp.hpp"
#include "co_texmng.hpp"
#include "co_fntmng.hpp"
#include "co_json.hpp"
#include "co_task.hpp"
#include "nn_camera.hpp"
#include "nn_earth.hpp"
#include "nn_light.hpp"
#include "nn_sound.hpp"
#include "nn_touchmenu.hpp"
#include "nn_settings.hpp"


namespace ngs {

struct GameEnv {
  Touch             *touch;
  Keyinp            *keyinp;
  Sound             *sound;
  const std::string *path;
  const std::string *savePath;
  Settings          *settings;
  Vec2<int>         *size;
  float              scale;                         // 画面のスケーリング(iPhone3Gなどで利用)
  bool               retina;
  float             *y_bottom;                      // iAd広告の用のオフセット
  Task::ProcPtr      touchMenu;
  Localize          *localize;

  Json   *params;
  Json   *places;
  std::map<std::string, Place> *place_data;
  FntMng *fonts;
  Task   *task;
  TexMng *texMng;
  Camera *camera;
  Camera *cockpit;
  Earth  *earth;
  Light  *earthLight;
  int earth_texture;

  // 以下プレイ中のデータ
  int   game_mode;
  float time;

  float ans_time;
  float ans_time_max;
  int   level;
  int   konahen;
  int   just;
  int   just_total;
  int   just_max;
  int   tap_num;
  int   miss_num;
  float total_time;                                 // プレイ時間
  int   score;
  int   rank;
  std::vector<float> q_time;                        // 一問ごとの回答時間

  int   question;
  int   question_total;
  int   konahen_cur;
  bool  started;
  bool  no_question;
  
  std::string  cur_place;
  bool         answer;
  std::string  ans_place;
  bool         onetime;
  std::string  cur_type;
  std::vector<Vec3<float> > place_center;
  Vec3<float>  hit_pos;
  bool         hit;
  bool         correct;

  bool         demo;
  u_int        replay_num;
  unsigned int rseed;

  bool         setup;                               // 本編開始
  bool         cleanup;                             // 後始末開始
};
// タスク間でのゲーム環境のやりとりを構造体で行うという試み

enum {
  TASK_PRIO_SYS,
  TASK_PRIO_3D_TOPPRIO,                             // 一番最初に描画するタスク(1つだけ存在)
  TASK_PRIO_3D,
  TASK_PRIO_2D_TOPPRIO,                             // 一番最初に描画するタスク(1つだけ存在)
  TASK_PRIO_2D,
};
// TIPS:3D系と2D系の描画でセットアップが違うのでタスクで切り分けた

enum {
  GAME_MODE_NORMAL = 0,
  GAME_MODE_ADVANCED,
  GAME_MODE_SURVIVAL,

  GAME_MODE_REVIEW,

  GAME_MODE_GETFULL,

  GAME_MODE_NUM
};

enum {
  MSG_GAME_CONTROL_START,                           // 操作開始
  MSG_GAME_CONTROL_STOP,                            // 操作停止
  MSG_CONTROL_LOCK,                                 // DEMO用に操作をロック
  MSG_CONTROL_UNLOCK,                               // DEMO終了時に操作再開
  MSG_CONTROL_XY,                                   // オイラー角で回転
  MSG_CONTROL_MIX,                                  // クオータニオンで回転
  
  MSG_TOUCH_START,                                  // タッチイベント開始
  MSG_TOUCH_END,                                    // タッチイベント終了
  MSG_TOUCH_MOVE,                                   // タッチ操作

  MSG_GAME_MAIN_INIT,
  MSG_GAME_START,                                   // プレイ開始
  MSG_GAME_END,                                     // プレイ終了
  MSG_GAME_CLEANUP,
  MSG_GAME_PAUSE,                                   // ポーズON/OFF
  
  MSG_GAME_TOUCH_START,
  MSG_GAME_TOUCH_STOP,
  
  MSG_GAME_TOUCH,
  MSG_GAME_TOUCH_IN,
  MSG_GAME_TOUCH_OUT,
  MSG_GAME_TOUCH_ANS,
  MSG_GAME_TOUCH_DISP,                              // 正解位置を強制的に表示
  MSG_GAME_TOUCH_BLINK_ON,                          // 点滅表示制御
  MSG_GAME_TOUCH_BLINK_OFF,                         // 点滅表示制御

  MSG_GAME_INTRO_END,
  MSG_GAME_GAMEOVER_END,
  MSG_GAME_RESULT_END,
  
  MSG_GAME_PLACEDISP_START,
  MSG_GAME_PLACEDISP_ANS,

  MSG_GAME_MENU_OFF,

  MSG_GAME_LEVELUP,
  MSG_GAME_HURRYUP,
  MSG_GAME_TIMESHORT,
  MSG_GAME_DOBEST,
  MSG_GAME_HINT,
  
  MSG_GAME_AGREE,
  
  MSG_TITLE_INTRO_FIN,
  MSG_TITLE_INTRO_SKIP,

  MSG_TITLE_CREDITS_PUSH,
  MSG_TITLE_CREDITS,
  MSG_TITLE_CREDITS_FIN,

  MSG_TITLE_SOUND_VOL_OPEN,
  MSG_TITLE_SOUND_VOL_CLOSE,

  MSG_TITLE_ERASE_OPEN,
  MSG_TITLE_ERASE_CLOSE,
  
  MSG_TITLE_EARTH_TEX_OPEN,
  MSG_TITLE_EARTH_TEX_CLOSE,
  MSG_TITLE_EARTH_TEX_CHANGE,

  MSG_TITLE_GAMESTART,
  MSG_TITLE_END,
  
  MSG_DEMOPLAY_END,
  MSG_RANKING_END,

  MSG_ACHIEVEMENT_END,

  MSG_FADELIGHT_STOP,
  
  MSG_RESETCAMERA_STOP,
  MSG_RESETCAMERA_ABORT,

  MSG_REVIEW_UPDATE,
  MSG_REVIEW_START,
  MSG_REVIEW_CONTROL_STOP,
  MSG_REVIEW_CONTROL_START,
};
// タスク間の通信用


}
