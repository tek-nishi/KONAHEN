
// #if defined (_MSC_VER)
// #ifdef _DEBUG
// #define _CRTDBG_MAP_ALLOC
// #include <cstdlib>
// #include <crtdbg.h>
// // #define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
// // #define new DEBUG_CLIENTBLOCK
// // TIPS:↑VSでメモリリーク箇所を表示する
// #endif
// #include <windows.h>
// #endif
// ↑Windowsはプリコンパイルしてるので要らない

#if defined (__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glext.h>
#endif

#include <string>
#include <memory>
#include <functional>
#include <ctime>

#ifndef _MSC_VER
#include <tr1/memory>
#include <tr1/functional>
// TIPS:OSXとiOSではTR1拡張はインクルードファイルが別
#endif

#include "co_defines.hpp"
#include "os_osx.hpp"
#include "os_win.hpp"
#include "os_linux.hpp"
#include "co_zlib.hpp"
#include "nn_app.hpp"
#include "nn_capture.hpp"

using namespace std;
using namespace std::tr1;
using namespace ngs;

#define APP_VERSION "1.1"
#define APP_TITLE   "KONAHEN"

enum {
  WINDOW_WIDTH    = 960,
  WINDOW_HEIGHT   = 640,
  FRAME_RATE      = 60,
  UPDATE_INTERVAL = 1000 / FRAME_RATE
};

int width_  = WINDOW_WIDTH;
int height_ = WINDOW_HEIGHT;
int win_pos_x_ = -1;
int win_pos_y_ = -1;

bool        left_btn;
bool        right_btn;
Vec2<float> l_mpos;
// ↑glutからの入力を適切に加工してアプリに渡す用

Os  *os;
App *app;                                           // アプリ本体

unsigned int frame_count;
unsigned int mouse_frame;

#ifdef _DEBUG
bool capture;
#endif

#if defined (_MSC_VER) && defined (_MEMORY_CHECK)

size_t mem_cur_;
size_t mem_max_;

void *operator new(size_t size)
{
  mem_cur_ += size;
  if (mem_max_ < mem_cur_)
  {
    mem_max_ = mem_cur_;
    DOUT << "new max:" << mem_max_ << ":" << size << endl;
  }
  else
  {
    DOUT << "new cur:" << mem_cur_ << ":" << size << endl;
  }
  return malloc(size);
}

void operator delete(void* p)
{
  size_t size = _msize(p);
  mem_cur_ -= size;
  free(p);
}

#endif


void timerCallback(const int msec)
{
  glutTimerFunc(msec, timerCallback, msec);
  glutPostRedisplay();
}

void resizeCallback(const int w, const int h)
{
  width_ = w;
  height_ = h;
  
  // app->resize(w, h);
  // app->resizeFit(w, h);
  app->resizeKeepAspect(w, h);
  // ↑各種拡大縮小処理
}

void displayCallback()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float delta_time = 1.0f / static_cast<float>(FRAME_RATE);
  app->update(delta_time);
  ++frame_count;
  glutSwapBuffers();

#ifdef _DEBUG
  const GLenum gl_err = glGetError();
  if (gl_err != GL_NO_ERROR)
  {
    const GLubyte *str = gluErrorString(gl_err);
    DOUT << "OpenGL Error:" << str << std::endl;
  }
  const ALenum al_err = alGetError();
  if (al_err != AL_NO_ERROR)
  {
    const char *str = AudioErrorString(al_err);
    DOUT << "OpenAL Error:" << str << std::endl;
  }

  if (capture)
  {
    std::string path = os->savePath() + "capture/";
    ExecCapture(path, width_, height_);
  }
#endif

  unsigned char key_inp = app->keyinp().get();
  if (key_inp == CTRL_R)
  {
    glutPositionWindow(win_pos_x_, win_pos_y_);
    glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
  }
  else
  if ((key_inp == CTRL_W) || (key_inp == CTRL_Q))
  {
    exit(0);
  }
#ifdef _DEBUG
  else
  if (key_inp == INP_KEY_F5)
  {
    DOUT << "Soft Reset" << endl;
    app->reset();
    resizeCallback(width_, height_);                // サイズ変更を実行しておく
  }
  else
  if (key_inp == 'S')
  {
    std::time_t t = std::time(NULL);
    tm  *ts = std::localtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "_%Y%m%d%H%M_%S", ts);
    std::stringstream sstr;
    sstr << "capture/spapshot" << buf << ".jpg" << std::flush;
    WriteFramebufferToJpegFile(sstr.str(), width_, height_);
    DOUT << "SnapShot!" << std::endl;
  }
  else
  if (key_inp == 'M')
  {
    capture = !capture;
    DOUT << "Movie capture:" << std::string(capture ? "START" : "END") << std::endl;

    app->forceFrame(capture);
    if (capture) StartCapture();
  }
  if (key_inp == 'I')
  {
    OutputGLStatus();
  }
#endif
}

void mouseCallBack(const int button, const int state, const int x, const int y)
{
  if (button == GLUT_LEFT_BUTTON)
  {
    left_btn = (state == GLUT_DOWN);
    if (!right_btn)
    {
      if (left_btn) l_mpos.set(x, y);

      Touch& touch = app->touch();
      std::vector<TouchInfo> v;
      TouchInfo info = {
        Vec2<float>(x, y),
        l_mpos
      };
      v.push_back(info);

      if (left_btn)
      {
        touch.touchStart(v);
      }
      else
      {
        touch.touchEnd(v);
      }
    }
  }
  else
  if (button == GLUT_RIGHT_BUTTON)
  {
    // 右クリック時は画面中心を挟んだマルチタッチを真似る
    right_btn = (state == GLUT_DOWN);
    if (right_btn) l_mpos.set(x, y);

    Touch& touch = app->touch();
    std::vector<TouchInfo> v;
    if (!left_btn)
    {
      TouchInfo info = {
        Vec2<float>(x, y),
        l_mpos
      };
      v.push_back(info);
    }
    TouchInfo info = {
      Vec2<float>(width_ - x, height_ - y),
      Vec2<float>(width_ - l_mpos.x, height_ - l_mpos.y)
    };
    v.push_back(info);
    
    if (right_btn)
    {
      touch.touchStart(v);
    }
    else
    {
      touch.touchEnd(v);
    }
  }
}

void motionCallback(const int x, const int y)
{
  Vec2<float> mpos = Vec2<float>(x, y);
  if (mouse_frame == frame_count)
  {
//    l_mpos = mpos;
    return;
  }
  mouse_frame = frame_count;
  // 同一フレームで二回以上処理しない
  
  Touch& touch = app->touch();
  TouchInfo info = {
    mpos,
    l_mpos
  };
  std::vector<TouchInfo> v;
  v.push_back(info);
  if (right_btn)
  {
    // 右クリック中は画面中心を挟んだマルチタッチを真似る
    TouchInfo info = {
      Vec2<float>(width_ - x, height_ - y),
      Vec2<float>(width_ - l_mpos.x, height_ - l_mpos.y),
    };
    v.push_back(info);
  }
  touch.touchMove(v);
  l_mpos = mpos;
}


void keyPushCallback(const u_char key, const int x, const int y)
{
  app->keyinp().pushCallback(key, x, y);
}

void keyPullCallback(const u_char key, const int x, const int y)
{
  app->keyinp().pullCallback(key, x, y);
}

void keyExPushCallback(const int key, const int x, const int y)
{
  app->keyinp().exPushCallback(key, x, y);
}

void keyExPullCallback(const int key, const int x, const int y)
{
  app->keyinp().exPullCallback(key, x, y);
}


void mainFin()
{
  DOUT << "Program fin!" << endl;
#if defined (_MSC_VER) && defined (_MEMORY_CHECK)
  DOUT << "Mem Cur/Max:" << mem_cur_ << "/" << mem_max_ << endl;
#endif
  
  delete app;                                       // TIPS:ヌルポインタをdeleteしても問題ない事が保障されている
  delete os;
}


int main(int argc, char **argv)
{
  os = new Os;
  // TIPS:↑ソフト終了時に明示的にデストラクタを呼び出すための措置
  
  // _CrtSetBreakAlloc(99);
  
  glutInit(&argc, argv);

  app = new App(WINDOW_WIDTH, WINDOW_HEIGHT, 1.0, os->path(), os->savePath(), os->lang());
  width_ = app->windowWidth();
  height_ = app->windowHeight();
  // TIPS:Windowのサイズをアプリ内で記録しているのでここで生成している

  int x = -1;
  int y = -1;
  int w = glutGet(GLUT_SCREEN_WIDTH);
  int h = glutGet(GLUT_SCREEN_HEIGHT);

  if(w > 0)
  {
    if (w < (width_ + 80))
    {
      width_ = w - 80;
    }
    x = (w - (width_ + 8)) / 2;                     // 若干のりしろを付けておく
    win_pos_x_ = (w - (WINDOW_WIDTH + 8)) / 2;      // 初期化時の位置
    
  }
  if(h > 0)
  {
    if (h < (height_ + 80))
    {
      height_ = h - 80;
    }
    y = (h - (height_ + 16)) / 2;                   // 若干のりしろを付けておく
    win_pos_y_ = (h - (WINDOW_HEIGHT + 16)) / 2;
  }
  
  glutInitWindowSize(width_, height_);
  glutInitWindowPosition(x, y);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow(APP_TITLE":"APP_VERSION);

  initGlexFunc();

  DOUT << "OpenGL ver:" << glGetString(GL_VERSION) << endl;
  DOUT << "OpenAL ver:" << hex << AL_VERSION << dec << endl;
  DOUT << "zlib ver:" << ZLIB_VERSION << endl;
  DOUT << "libpng ver:" <<  PNG_LIBPNG_VER_STRING << endl;
  DOUT << "freetype ver:" << FREETYPE_MAJOR << "." << FREETYPE_MINOR << "." << FREETYPE_PATCH << endl;

  app->reset();                                     // 最初に動作するプロセスを生成
  int intarval_time = (os->isVsyncSwap() && os->toggleVsyncSwap(1)) ? UPDATE_INTERVAL - 1 : UPDATE_INTERVAL;
  DOUT << "intarval_time:" << intarval_time << endl;

  atexit(mainFin);

  glutDisplayFunc(displayCallback);
  glutReshapeFunc(resizeCallback);

  glutMouseFunc(mouseCallBack);
  glutMotionFunc(motionCallback);
  // glutPassiveMotionFunc(motionCallback);
  // ↑タッチ操作では画面に指を触れないとマウスカーソルを動かせないので、これはいらない

  glutKeyboardFunc(keyPushCallback);
  glutKeyboardUpFunc(keyPullCallback);
  glutSpecialFunc(keyExPushCallback);
  glutSpecialUpFunc(keyExPullCallback);

  glHint(GL_FOG_HINT, GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  // ↑たいてい無視されている模様:P

  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  // 最初に設定してしまえばいいもの

  glutTimerFunc(0, timerCallback, intarval_time);
  glutMainLoop();

  return 0;
}
