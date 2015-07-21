
#pragma once

#include <cmath>
#include <string>
#include <vector>
#include "co_vec2.hpp"
#include "co_vec3.hpp"
#include "co_quat.hpp"
#include "co_matrix.hpp"
#include "co_misc.hpp"
#include "co_texmng.hpp"
#include "co_fntmng.hpp"
#include "co_easearray.hpp"
#include "co_jpeg.hpp"


namespace ngs {

// 3D→経度・緯度
Vec2<float> posToLoc(const Vec3<float>& pos)
{
	Vec3<float> v = pos;
	v.unit();

	float y = asin(v.y);
	float r = sqrt(1.0 - v.y * v.y);
	float x = (r > 0.0) ? asin(v.z / r) : 0.0;
	if(v.x > 0.0) x = ((x > 0) ? PI : -PI) - x;

	return Vec2<float>(x, y);
}

// 経度・緯度→2D
Vec2<float> locToPos2d(const Vec2<float>& loc)
{
	float x = loc.x / -PI;
	float y = loc.y / (-PI / 2.0);

	return Vec2<float>(x, y);
}

// 経度・緯度→3D
Vec3<float> locToPos3d(const Vec2<float>& loc)
{
	float y = sin(loc.y);
	float r = cos(loc.y);
	float z = sin(loc.x) * r;
	float x = cos(loc.x) * r;
		
	return Vec3<float>(-x, y, z);
}

// 2D→3D
Vec3<float> locToPos(const Vec2<float>& pos)
{
	float x = pos.x * -PI;
	float y = pos.y * (-PI / 2.0);
	return locToPos3d(Vec2<float>(x, y));
}

// クオータニオン→オイラー角
Vec2<float> QuatToAngle(const Quat<float>& rot)
{
	Matrix mat;
	mat.rotate(rot);
	const GLdouble *m = mat.value();
	float rx = -asin(minmax<float>(-m[6], -1.0, 1.0));
	// TIPS:X軸の向きは±90以内に収まっているので計算を省略できる

	float r = acos(minmax<float>(m[0], -1.0, 1.0));
	float ry = (-m[2] < 0.0) ? -r : r;
	return Vec2<float>(rx, ry);
}


// テクスチャに縦横に等幅で並んだ数字を描画
void DrawNumber(const int num, const Vec2<float>& pos, const Vec2<int>& size, const Vec2<float>& uv, const GrpCol<float>& col, const TexMng::tex_ptr texture)
{
	float ox = (num / 5) * size.x;
	float oy = (num % 5) * size.y;
	
	GrpSprite obj;
	obj.size(size.x, size.y);
	obj.pos(pos);
	obj.texture(texture);
	obj.uv(uv.x + ox, uv.y + oy);
	obj.col(col);
	obj.draw();
}

// 指定桁で表示
void DrawNumberPlace(const int num, const int place, const float x_ofs, const Vec2<float>& pos, const Vec2<int>& size, const Vec2<float>& uv, const GrpCol<float>& col, const TexMng::tex_ptr texture)
{
	Vec2<float> p(pos);
	int val = std::pow(10.0, place);
	int n = num;
	while(val > 1)
	{
		n = n % val;
		val /= 10;
		DrawNumber(n / val, p, size, uv, col, texture);
		p.x += x_ofs;
	}
}

// 決め打ちで数字を表示
void DrawNumberBig(const int num, const int place, const Vec2<float>& pos, const GrpCol<float>& col, const TexMng::tex_ptr texture)
{
	Vec2<int> size(43, 69);
	Vec2<float> uv(2, 2);
	if (place == 1)
	{
		DrawNumber(num, pos, size, uv, col, texture);
	}
	else
	{
		DrawNumberPlace(num, place, 42, pos, size, uv, col, texture);
	}
}

void DrawNumberSmall(const int num, const int place, const Vec2<float>& pos, const GrpCol<float>& col, const TexMng::tex_ptr texture)
{
	Vec2<int> size(25, 40);
	Vec2<float> uv(132, 2);
	if (place == 1)
	{
		DrawNumber(num, pos, size, uv, col, texture);
	}
	else
	{
		DrawNumberPlace(num, place, 22, pos, size, uv, col, texture);
	}
}


// Fontを読む
FntMng::FontPtr ReadFont(const std::string& name, FntMng& fonts, const std::string& path, picojson::object& params)
{
	picojson::object& obj = params["fonts"].get<picojson::object>()[name].get<picojson::object>();
	const std::string f = path + obj["file"].get<std::string>();
	int type = FONT_TEXTURE;
	if (obj["buffer"].is<bool>() && obj["buffer"].get<bool>())
	{
		type = FONT_BUFFER;
	}
	return fonts.read(name, obj["height"].get<double>(), f, type);
}

// 事前にフォントを読む
void PreReadFont(FntMng& fonts, const std::string& path, picojson::object& params)
{
	picojson::object& obj = params["fonts"].get<picojson::object>();
	for(picojson::object::iterator it = obj.begin(); it != obj.end(); ++it)
	{
		ReadFont(it->first, fonts, path, params);
	}
}


void EasingArayFloat(const int type, const picojson::array& easeing, EaseArray<float>& array, const bool addition = false)
{
	if (!addition) array.clear();
	for(picojson::array::const_iterator it = easeing.begin(); it != easeing.end(); ++it)
	{
		const picojson::array& p = it->get<picojson::array>();
		array.push_back(type, p[0].get<double>(), p[1].get<double>(), p[2].get<double>());
	}
}

void EasingArayVec2(const int type, const picojson::array& easeing, EaseArray<Vec2<float> >& array, const bool addition = false)
{
	if (!addition) array.clear();
	for(picojson::array::const_iterator it = easeing.begin(); it != easeing.end(); ++it)
	{
		const picojson::array& p = it->get<picojson::array>();
		array.push_back(type,
										Vec2<float>(p[0].get<double>(), p[1].get<double>()),
										Vec2<float>(p[2].get<double>(), p[3].get<double>()),
										p[4].get<double>());
	}
}

void EasingArayVec3(const int type, const picojson::array& easeing, EaseArray<Vec3<float> >& array, const bool addition = false)
{
	if (!addition) array.clear();
	for(picojson::array::const_iterator it = easeing.begin(); it != easeing.end(); ++it)
	{
		const picojson::array& p = it->get<picojson::array>();
		array.push_back(type,
										Vec3<float>(p[0].get<double>(), p[1].get<double>(), p[2].get<double>()),
										Vec3<float>(p[3].get<double>(), p[4].get<double>(), p[5].get<double>()),
										p[6].get<double>());
	}
}

void EasingArayVec4(const int type, const picojson::array& easeing, EaseArray<Vec4<float> >& array, const bool addition = false)
{
	if (!addition) array.clear();
	for(picojson::array::const_iterator it = easeing.begin(); it != easeing.end(); ++it)
	{
		const picojson::array& p = it->get<picojson::array>();
		array.push_back(type,
										Vec4<float>(p[0].get<double>(), p[1].get<double>(), p[2].get<double>(), p[3].get<double>()),
										Vec4<float>(p[4].get<double>(), p[5].get<double>(), p[6].get<double>(), p[7].get<double>()),
										p[8].get<double>());
	}
}


// 書き出したparamファイルの存在判定
bool isSavedParamExists(const std::string& path)
{
#ifdef WRITE_TO_Z
	std::string::size_type index = path.rfind(".json");
	if(index != std::string::npos)
	{
		std::string res = path.substr(0, index) + ".dz";
		return isFileExists(res);
	}
#endif
	return isFileExists(path);
}


// ランダム
// SOURCE:http://d.hatena.ne.jp/faith_and_brave/20080121/1200911637
template <class Engine, class ArgumentType=long>
class random_number_generator {
	Engine& engine_;
public:
	random_number_generator(Engine& engine) : engine_(engine) {}

	ArgumentType operator()(ArgumentType value)
	{
		return engine_() % value;
	}
};

template <class Iterator, class Engine>
inline void random_shuffle_for_engine(Iterator first, Iterator last, Engine& engine)
{
	random_number_generator<Engine> gen(engine);
	std::random_shuffle(first, last, gen);
}


#ifdef _DEBUG

//
// フレームバッファをファイルに書き出す
//
void WriteFramebufferToPngFile(const std::string& file, u_int width, u_int height)
{
	GLint pack, unpack;
	glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// ↑FIXME:これらの設定が必要なのか微妙

		// glReadBuffer(GL_FRONT);
	std::vector<u_char> pixcel(width * height * 4);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixcel[0]);
	WritePng(file, width, height, &pixcel[0]);

	glPixelStorei(GL_PACK_ALIGNMENT, pack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
}

void WriteFramebufferToJpegFile(const std::string& file, u_int width, u_int height)
{
	GLint pack, unpack;
	glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// ↑FIXME:これらの設定が必要なのか微妙

		// glReadBuffer(GL_FRONT);
	std::vector<u_char> pixcel(width * height * 4);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixcel[0]);
	WriteJpeg(file, width, height, &pixcel[0]);

	glPixelStorei(GL_PACK_ALIGNMENT, pack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
}


// 
// OpenGLの状態を出力
//
void OutputGLStatus()
{
	enum {
		GLST_BOOL,
		GLST_DOUBLE,
		GLST_FLOAT,
		GLST_INT,
		GLST_FLAG
	};
	struct StateGL {
		GLenum pname;
		u_int type;
		u_int num;
		const char *name;
	};

	static const StateGL lists[] = {
		{
			GL_CURRENT_COLOR,
			GLST_FLOAT, 4,
			"GL_CURRENT_COLOR"
		},
		{
			GL_FOG,
			GLST_FLAG, 1,
			"GL_FOG"
		},
		{
			GL_LIGHTING,
			GLST_FLAG, 1,
			"GL_LIGHTING"
		},
		{
			GL_ALPHA_TEST,
			GLST_FLAG, 1,
			"GL_ALPHA_TEST"
		},
		{
			GL_BLEND,
			GLST_FLAG, 1,
			"GL_BLEND"
		}
	};

	for (u_int i = 0; i < elemsof(lists); ++i)
	{
		switch(lists[i].type)
		{
		case GLST_BOOL:
			{
				GLboolean *params = new GLboolean[lists[i].num];
				glGetBooleanv(lists[i].pname, params);
				DOUT << lists[i].name << ":";
				for (u_int h = 0; h < lists[i].num; ++h)
				{
					DOUT << *(params + h) << " ";
				}
				DOUT << std::endl;
				delete[] params;
			}
			break;

		case GLST_DOUBLE:
			{
				GLdouble *params = new GLdouble[lists[i].num];
				glGetDoublev(lists[i].pname, params);
				DOUT << lists[i].name << ":";
				for (u_int h = 0; h < lists[i].num; ++h)
				{
					DOUT << *(params + h) << " ";
				}
				DOUT << std::endl;
				delete[] params;
			}
			break;

		case GLST_FLOAT:
			{
				GLfloat *params = new GLfloat[lists[i].num];
				glGetFloatv(lists[i].pname, params);
				DOUT << lists[i].name << ":";
				for (u_int h = 0; h < lists[i].num; ++h)
				{
					DOUT << *(params + h) << " ";
				}
				DOUT << std::endl;
				delete[] params;
			}
			break;

		case GLST_INT:
			{
				GLint *params = new GLint[lists[i].num];
				glGetIntegerv(lists[i].pname, params);
				DOUT << lists[i].name << ":";
				for (u_int h = 0; h < lists[i].num; ++h)
				{
					DOUT << *(params + h) << " ";
				}
				DOUT << std::endl;
				delete[] params;
			}
			break;

		case GLST_FLAG:
			{
				GLboolean flag = glIsEnabled(lists[i].pname);
				DOUT << lists[i].name << ":" << (flag ? "ON" : "OFF") << std::endl;
			}
			break;
		}
	}
}

#endif


#if (TARGET_OS_IPHONE)

// 
// Mesa-7.11から引用
// SOURCE:http://www.mesa3d.org/
// 

void __gluMultMatricesd(const GLdouble a[16], const GLdouble b[16],GLdouble r[16])
{
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
	    r[i*4+j] = 
				a[i*4+0]*b[0*4+j] +
				a[i*4+1]*b[1*4+j] +
				a[i*4+2]*b[2*4+j] +
				a[i*4+3]*b[3*4+j];
		}
	}
}

int __gluInvertMatrixd(const GLdouble m[16], GLdouble invOut[16])
{
	GLdouble inv[16], det;
	int i;

	inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
		+ m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
		- m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
		+ m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
		- m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
		- m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
		+ m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
		- m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
		+ m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
		+ m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
	inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
		- m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
	inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
		+ m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
		- m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
	inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
		- m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
	inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
		+ m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
		- m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
	inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
		+ m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (det == 0)
	return GL_FALSE;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
	invOut[i] = inv[i] * det;

	return GL_TRUE;
}

void __gluMultMatrixVecd(const GLdouble matrix[16], const GLdouble in[4], GLdouble out[4])
{
	int i;

	for (i=0; i<4; ++i) {
		out[i] = 
	    in[0] * matrix[0*4+i] +
	    in[1] * matrix[1*4+i] +
	    in[2] * matrix[2*4+i] +
	    in[3] * matrix[3*4+i];
	}
}

GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
									 const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4],
									 GLdouble *objx, GLdouble *objy, GLdouble *objz)
{
	GLdouble finalMatrix[16];
	GLdouble in[4];
	GLdouble out[4];

	__gluMultMatricesd(modelMatrix, projMatrix, finalMatrix);
	if (!__gluInvertMatrixd(finalMatrix, finalMatrix)) return(GL_FALSE);

	in[0]=winx;
	in[1]=winy;
	in[2]=winz;
	in[3]=1.0;

	/* Map x and y from window coordinates */
	in[0] = (in[0] - viewport[0]) / viewport[2];
	in[1] = (in[1] - viewport[1]) / viewport[3];

	/* Map to range -1 to 1 */
	in[0] = in[0] * 2 - 1;
	in[1] = in[1] * 2 - 1;
	in[2] = in[2] * 2 - 1;

	__gluMultMatrixVecd(finalMatrix, in, out);
	if (out[3] == 0.0) return(GL_FALSE);
	out[0] /= out[3];
	out[1] /= out[3];
	out[2] /= out[3];
	*objx = out[0];
	*objy = out[1];
	*objz = out[2];
	return(GL_TRUE);
}

GLint gluProject(GLdouble objx, GLdouble objy, GLdouble objz, 
								 const GLdouble modelMatrix[16], 
								 const GLdouble projMatrix[16],
								 const GLint viewport[4],
								 GLdouble *winx, GLdouble *winy, GLdouble *winz)
{
	GLdouble in[4];
	GLdouble out[4];

	in[0]=objx;
	in[1]=objy;
	in[2]=objz;
	in[3]=1.0;
	__gluMultMatrixVecd(modelMatrix, in, out);
	__gluMultMatrixVecd(projMatrix, out, in);
	if (in[3] == 0.0) return(GL_FALSE);
	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];
	/* Map x, y and z to range 0-1 */
	in[0] = in[0] * 0.5 + 0.5;
	in[1] = in[1] * 0.5 + 0.5;
	in[2] = in[2] * 0.5 + 0.5;

	/* Map x,y to viewport */
	in[0] = in[0] * viewport[2] + viewport[0];
	in[1] = in[1] * viewport[3] + viewport[1];

	*winx=in[0];
	*winy=in[1];
	*winz=in[2];
	return(GL_TRUE);
}


struct token_string
{
	GLuint Token;
	const char *String;
};

static const struct token_string Errors[] = {
	{ GL_NO_ERROR, "no error" },
	{ GL_INVALID_ENUM, "invalid enumerant" },
	{ GL_INVALID_VALUE, "invalid value" },
	{ GL_INVALID_OPERATION, "invalid operation" },
	{ GL_STACK_OVERFLOW, "stack overflow" },
	{ GL_STACK_UNDERFLOW, "stack underflow" },
	{ GL_OUT_OF_MEMORY, "out of memory" },
	{ static_cast<GLuint>(~0), "Unknown Error" } /* end of list indicator */
};

const GLubyte* gluErrorString(GLenum errorCode)
{
	int i;
	for (i = 0; i < elemsof(Errors) - 1; ++i) {
		if (Errors[i].Token == errorCode)
		return (const GLubyte *) Errors[i].String;
	}
	return (const GLubyte *) Errors[i].String;
}

#endif

}
