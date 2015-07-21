
#pragma once

//
// 事前定義
//

// #define VER_LITE

#ifdef _DEBUG
#define NO_READ_PLACES                              // 問題を先読みしない
#define NN_APP_DISP_FPS                             // FPS表示
// #define REPLAY_REC                                 // デモ用にプレイを記録
#define JSON_CONV                                   // JSONを圧縮
#define ANY_CORRECT                                 // どこをタッチしても正解
// #define _MEMORY_CHECK                                // メモリ使用量をチェック
#define TOUCH_RECORD                                // 操作を記録する
#endif

#define READ_FROM_Z                                 // zlib圧縮テキスト対応
#ifdef READ_FROM_Z
#define WRITE_TO_Z
#endif

#ifdef _DEBUG
#define DOUT std::cout
#else
#define DOUT 0 && std::cout
#endif
// std::cout一網打尽マクロ

const unsigned int osTimerPeriod = 1;               // WinのtimeBeginPeriod()で使う

#if defined (PI)
#undef PI
#endif
const float PI = 3.1415926535897932384626433832795028841968;

#define elemsof(a)  ((int)(sizeof(a) / sizeof((a)[0])))

typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;

#if (TARGET_OS_IPHONE)

#ifdef _DEBUG
#define NSLOG(...) NSLog(__VA_ARGS__)
#else
#define NSLOG(...) 
#endif


typedef GLfloat GLdouble;
// FIXME:ちょっと微妙

#define glFrustum(left, right, bottom, top, znear, zfar)  glFrustumf(left, right, bottom, top, znear, zfar)
#define glOrtho(left, right, bottom, top, znear, zfar)  glOrthof(left, right, bottom, top, znear, zfar)
#define glTranslated(x, y, z)  glTranslatef(x, y, z)
#define glGetDoublev(pname, params)  glGetFloatv(pname, params)
#define glLightModeli(pname, params)  glLightModelx(pname, params)
#define glLoadMatrixd(mtx)  glLoadMatrixf(mtx)
#define glMultMatrixd(mtx)  glMultMatrixf(mtx)
#define gluBuild2DMipmaps(target, internalFormat, width, height, format, type, data)  glTexImage2D(target, 0, internalFormat, width, height, 0, format, type, data)
#define glFogi(pname, param) glFogx(pname, param)
#define glPushClientAttrib(param)
#define glPopClientAttrib() { glDisableClientState(GL_VERTEX_ARRAY); glDisableClientState(GL_NORMAL_ARRAY); glDisableClientState(GL_TEXTURE_COORD_ARRAY); glDisableClientState(GL_COLOR_ARRAY); }
// FIXME:↑プリプロセッサを使わない実装


enum {
  GLUT_KEY_F1  = 1,
  GLUT_KEY_F2  = 2,
  GLUT_KEY_F3  = 3,
  GLUT_KEY_F4  = 4,
  GLUT_KEY_F5  = 5,
  GLUT_KEY_F6  = 6,
  GLUT_KEY_F7  = 7,
  GLUT_KEY_F8  = 8,
  GLUT_KEY_F9  = 9,
  GLUT_KEY_F10 = 10,
  GLUT_KEY_F11 = 11,
  GLUT_KEY_F12 = 12,
  
  GLUT_KEY_LEFT      = 100,
  GLUT_KEY_UP        = 101,
  GLUT_KEY_RIGHT     = 102,
  GLUT_KEY_DOWN      = 103,
  GLUT_KEY_PAGE_UP   = 104,
  GLUT_KEY_PAGE_DOWN = 105,
  GLUT_KEY_HOME      = 106,
  GLUT_KEY_END       = 107,
  GLUT_KEY_INSERT    = 108,
};

#define glutIgnoreKeyRepeat(aValue)
// FIXME:↑プリプロセッサを使わない実装

#endif

#if defined (_MSC_VER)
#pragma execution_character_set("utf-8")
// TIPS:VC2010だとこうしとけば文字リテラルのエンコードを指定できる
#endif
