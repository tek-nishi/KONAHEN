// GLUT
#define HAVE_GL_GLUT_H

#ifdef _MSC_VER

// M_PI and friends on VC
#define _USE_MATH_DEFINES

// quell spurious "'this': used in base member initializer list" warnings
#pragma warning(disable: 4355)

#endif

// quell spurious portable-function deprecation warnings
#define _CRT_SECURE_NO_DEPRECATE 1
#define _POSIX_ 1
