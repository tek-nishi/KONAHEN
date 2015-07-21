
#pragma once

/* Windows専用プリコンパイル用ヘッダ */

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif
#include <windows.h>
#include <mmsystem.h>

#include <GL/glut.h>
#include <GL/glext.h>

#include <AL/al.h>
#include <AL/alc.h>

#include <random>
#include <memory>
#include <functional>
#include <locale>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <zlib.h>
#include <png.h>
#include <jpeglib.h>
#define FTGL_LIBRARY_STATIC
#include <FTGL/ftgl.h>
#include "picojson.h"
