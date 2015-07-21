
#pragma once

//
// OpenGL拡張機能管理
//


namespace ngs {

bool use_glex_vbo;
bool use_glex_blend;
bool use_glex_muitl;

#if defined (_MSC_VER)

PFNGLGENBUFFERSPROC          glGenBuffers;
PFNGLBINDBUFFERPROC          glBindBuffer;
PFNGLBUFFERDATAPROC          glBufferData;
PFNGLDELETEBUFFERSPROC       glDeleteBuffers;
PFNGLBLENDEQUATIONPROC       glBlendEquation;
PFNGLACTIVETEXTUREPROC       glActiveTexture;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;

// glutCreateWindow()した後で実行する事
void initGlexFunc()
{
  bool res = true;
  
  if ((glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers")) == 0) res = false;
  if ((glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer")) == 0) res = false;
  if ((glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData")) == 0) res = false;
  if ((glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers")) == 0) res = false;

  DOUT << "VBO:" << res << std::endl;

  use_glex_vbo = res;

  use_glex_blend = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation")) == 0) ? false : true;
  DOUT << "Blend:" << use_glex_blend << std::endl;

  res = true;
  if ((glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture")) == 0) res = false;
  if ((glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture")) == 0) res = false;
  use_glex_muitl = res;
}

#elif defined (__APPLE__)

void initGlexFunc()
{
  use_glex_vbo = true;
  use_glex_blend = true;
  use_glex_muitl = true;
  // Apple製品は全部使える
}

#else

PFNGLGENBUFFERSPROC          glGenBuffers;
PFNGLBINDBUFFERPROC          glBindBuffer;
PFNGLBUFFERDATAPROC          glBufferData;
PFNGLDELETEBUFFERSPROC       glDeleteBuffers;
PFNGLBLENDEQUATIONPROC       glBlendEquation;
PFNGLACTIVETEXTUREPROC       glActiveTexture;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;

void initGlexFunc()
{
  bool res = false;

  if ((glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenBuffers")) == 0) res = false;
  if ((glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindBuffer")) == 0) res = false;
  if ((glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte *)"glBufferData")) == 0) res = false;
  if ((glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glBufferData")) == 0) res = false;

  DOUT << "VBO:" << res << std::endl;
  use_glex_vbo = res;

  use_glex_blend = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glXGetProcAddress((const GLubyte *)"glBlendEquation")) == 0) ? false : true;
  DOUT << "Blend:" << use_glex_blend << std::endl;

  res = true;
  if ((glActiveTexture = (PFNGLACTIVETEXTUREPROC)glXGetProcAddress((const GLubyte *)"glActiveTexture")) == 0) res = false;
  if ((glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glXGetProcAddress((const GLubyte *)"glClientActiveTexture")) == 0) res = false;
  use_glex_muitl = res;
}

#endif

}
