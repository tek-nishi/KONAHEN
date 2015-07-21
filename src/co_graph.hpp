
#pragma once

//
// 描画基本クラス
// TODO: 3d拡張
// 

#include "co_vec2.hpp"
#include "co_texmng.hpp"
#include "co_misc.hpp"
#include <vector>

namespace ngs {

enum enmGRP_BLEND {
	GRP_BLEND_NORMAL,							// 通常のブレンディング
	GRP_BLEND_ADD,								// 加算半透明
	GRP_BLEND_REV,								// 反転表示
	GRP_BLEND_XOR,								// XOR
	GRP_BLEND_MUL,								// 乗算
	GRP_BLEND_SCREEN,							// スクリーン合成
};

void GrpSetBlend(const int blend)
{
	switch(blend)
	{
	case GRP_BLEND_NORMAL:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;

	case GRP_BLEND_ADD:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;

	case GRP_BLEND_REV:
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		break;

	case GRP_BLEND_XOR:
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
		break;

	case GRP_BLEND_MUL:
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		break;

	case GRP_BLEND_SCREEN:
		glBlendFunc(GL_DST_COLOR, GL_ONE);
		break;
	}
}


template <typename T>
class GrpCol {
public:
	T r, g, b, a;
	GrpCol() :
		r(),
		g(),
		b(),
		a(static_cast<T>(1))
	{}
	GrpCol(T red, T green, T blue, T alpha) :
		r(red),
		g(green),
		b(blue),
		a(alpha)
	{}

	void set(T red, T green, T blue, T alpha = static_cast<T>(1))
	{
		r = red, g = green, b = blue, a = alpha;
	}
	// TODO:カラー演算
	// 整数と小数でmaxを変える
};
	

class GrpBase
{
protected:
	int blend_;
	Vec2<float> pos_;
	Vec2<float> center_;
	Vec2<float> scale_;
	GrpCol<float> col_;
	float rotate_;

	void set_mtx() const {
		glPushMatrix();
		glTranslatef(pos_.x, pos_.y, 0);
		glRotatef(Rad2Ang(rotate_), 0, 0, 1);
		glScalef(scale_.x, scale_.y, 1.0);
		glTranslatef(-center_.x, -center_.y, 0);
	}

public:
	GrpBase() :
		blend_(GRP_BLEND_NORMAL),
		scale_(1.0f, 1.0f),
		rotate_()
	{}
	virtual ~GrpBase() {}

	void blend(const int blend) { blend_ = blend; }
	void col(const GrpCol<float> col) { col_ = col; }
	void col(const float r, const float g, const float b, const float a = 1.0f) {
		col_.set(r, g, b, a);
	}

	void pos(const Vec2<float>& pos) { pos_ = pos; }
	void pos(const float x, const float y) { pos_.set(x, y); }

	void center(const Vec2<float>& center) { center_ = center; }
	void center(const float x, const float y) { center_.set(x, y); }

	void scale(const Vec2<float>& scale) { scale_ = scale; }
	void scale(const float x, const float y) { scale_.set(x, y); }

	void rotate(const float rotate) { rotate_ = rotate; }

	virtual void draw() {}
};


class GrpPoint : public GrpBase {
	bool smooth_;
	float size_;
public:
	GrpPoint() :
		smooth_(),
		size_(1.0f)
	{}

	void smooth(const bool smooth) { smooth_ = smooth; }
	void size(const float size) { size_ = size; }

	virtual void draw()
	{
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		glPointSize(size_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		GLfloat vtx[] = { 0.0f, 0.0f };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_POINTS, 0, 1);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpLine : public GrpBase
{
	Vec2<float> ed_;
	bool smooth_;
	float width_;
public:
	GrpLine() :
		smooth_(),
		width_(1.0f)
	{}
		
	void points(const Vec2<float>& st, const Vec2<float>& ed)
	{
		pos_ = st;
		ed_ = ed - st;
	}
	void points(const float st_x, const float st_y, const float ed_x, const float ed_y) {
		pos_.set(st_x, st_y);
		ed_.set(ed_x - st_x, ed_y - st_y);
	}
		
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
	void width(const float width) {
		width_ = width;
	}

	virtual void draw() {
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		glLineWidth(width_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		GLfloat vtx[] = { 0.0f, 0.0f, ed_.x, ed_.y };
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpBox : public GrpBase {
	Vec2<float> size_;
	bool smooth_;
	float width_;
	bool fill_;
public:
	GrpBox() :
		smooth_(),
		width_(1.0f),
		fill_()
	{}
		
	void size(const Vec2<float>& size) {
		size_ = size;
	}
	void size(const float x, const float y) {
		size_.set(x, y);
	}
		
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
	void width(const float width) {
		width_ = width;
	}
	void fill(const bool fill) {
		fill_ = fill;
	}

	virtual void draw() {
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		glLineWidth(width_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		GLfloat vtx[] = {
			0.0f, 0.0f,
			0.0f, size_.y,
			size_.x, 0.0f,
			size_.x, size_.y,
		};
			
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(fill_ ? GL_TRIANGLE_STRIP : GL_LINE_LOOP, 0, 4);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpCircle : public GrpBase {
	Vec2<float> radius_;
	bool smooth_;
	float width_;
	bool fill_;
	int div_;
public:
	GrpCircle() :
		smooth_(),
		width_(1.0f),
		fill_(),
		div_(10)
	{}

	void radius(const Vec2<float>& radius) {
		radius_ = radius;
	}
	void radius(const float x, const float y) {
		radius_.set(x, y);
	}

	void div(int div) {
		div_ = div;
	}
	void width(const float width) {
		width_ = width;
	}
	void fill(const bool fill) {
		fill_ = fill;
	}
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
		
	virtual void draw()
	{
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		if (!fill_) glLineWidth(width_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		std::vector<GLfloat> vtx;
		vtx.reserve(div_ * 2);
		if(fill_)
		{
			vtx.push_back(0.0);
			vtx.push_back(0.0);
			for(int i = 0; i <= div_; ++i)
			{
				float r;
				r = (PI * 2.0f * (float)i) / div_;
				vtx.push_back(radius_.x * sin(r));
				vtx.push_back(radius_.y * cos(r));
			}
		}
		else
		{
			for(int i = 0; i < div_; ++i)
			{
				float r;
				r = (PI * 2.0f * (float)i) / div_;
				vtx.push_back(radius_.x * sin(r));
				vtx.push_back(radius_.y * cos(r));
			}
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, &vtx[0]);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(fill_ ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, vtx.size() / 2);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpDashedCircle : public GrpBase {
	Vec2<float> radius_;
	bool smooth_;
	float width_;
	int div_;
public:
	GrpDashedCircle() :
		smooth_(),
		width_(1.0f),
		div_(10)
	{}

	void radius(const Vec2<float>& radius) {
		radius_ = radius;
	}
	void radius(const float x, const float y) {
		radius_.set(x, y);
	}

	void div(int div) {
		div_ = div;
	}
	void width(const float width) {
		width_ = width;
	}
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
		
	virtual void draw()
	{
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		glLineWidth(width_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		std::vector<GLfloat> vtx;
		vtx.reserve(div_ * 2);
		for(int i = 0; i < div_; ++i)
		{
			float r;
			r = (PI * 2.0f * (float)i) / div_;
			vtx.push_back(radius_.x * sin(r));
			vtx.push_back(radius_.y * cos(r));

			r += (PI * 2.0f * 0.5f) / div_;
			vtx.push_back(radius_.x * sin(r));
			vtx.push_back(radius_.y * cos(r));
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, &vtx[0]);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_LINES, 0, vtx.size() / 2);
		glPopMatrix();
	}
};

class GrpTri : public GrpBase {
	Vec2<float> points_[3];
public:
	GrpTri() {}
		
	void points(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3) {
		points_[0] = p1;
		points_[1] = p2;
		points_[2] = p3;
	}

	virtual void draw() {
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		this->set_mtx();

		GLfloat vtx[] = {
			points_[0].x, points_[0].y,
			points_[1].x, points_[1].y,
			points_[2].x, points_[2].y
		};
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpFan : public GrpBase {
	Vec2<float> radius_;
	Vec2<float> uv_top_, uv_bottom_;
	bool smooth_;
	float width_;
	bool fill_;
	bool texture_;
	TexMng::tex_ptr tex_;
	int div_;
	float angle_;
public:
	GrpFan() :
		smooth_(),
		width_(1.0f),
		fill_(),
		texture_(),
		div_(10)
	{}

	void radius(const Vec2<float>& radius) {
		radius_ = radius;
	}
	void radius(const float x, const float y) {
		radius_.set(x, y);
	}

	void div(int div) {
		div_ = div;
	}
	void angle(float angle) {
		angle_ = angle;
	}
	void width(const float width) {
		width_ = width;
	}
	void fill(const bool fill) {
		fill_ = fill;
	}
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
	void texture(const TexMng::tex_ptr tex) {
		tex_ = tex;
		texture_ = true;
	}
	void uv(const float top_x, const float top_y, const float bottom_x, const float bottom_y) {
		uv_top_.set(top_x, top_y);
		uv_bottom_.set(bottom_x, bottom_y);
	}
		
	virtual void draw() {
		if (texture_)
		{
			glEnable(GL_TEXTURE_2D);
			tex_->bind();
			GLint filter = smooth_ ? GL_LINEAR : GL_NEAREST;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			GrpSetBlend(blend_);
			glLineWidth(width_);
			if(smooth_) glEnable(GL_LINE_SMOOTH);
			else		    glDisable(GL_LINE_SMOOTH);
		}
		this->set_mtx();

		std::vector<GLfloat> vtx;
		vtx.reserve(div_ * 2);
		vtx.push_back(0.0);
		vtx.push_back(0.0);
		for (int i = 0; i <= div_; ++i)
		{
			float r = (angle_ * (float)i) / (float)div_;
			vtx.push_back(radius_.x * sin(r));
			vtx.push_back(radius_.y * cos(r));
		}

		std::vector<GLfloat> uv_vtx;
		uv_vtx.reserve(div_ * 2);
		if (texture_)
		{
			float top_u, top_v;
			float bottom_u, bottom_v;
			const Vec2<int>& size = tex_->size();
			const Vec2<float>& uv = tex_->uv();
			
			top_u = uv_top_.x * uv.x / size.x;
			top_v = uv_top_.y * uv.y / size.y;
			bottom_u = uv_bottom_.x * uv.x / size.x;
			bottom_v = uv_bottom_.y * uv.y / size.y;
			uv_vtx.push_back(top_u);
			uv_vtx.push_back(top_v);
			for (int i = 0; i <= div_; ++i)
			{
				uv_vtx.push_back(bottom_u);
				uv_vtx.push_back(bottom_v);
			}
		}
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, &vtx[0]);
		if (texture_)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, &uv_vtx[0]);
		}
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(fill_ ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, vtx.size() / 2);
		if (texture_) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpDounut : public GrpBase {
	Vec2<float> radius_, hole_;
	int div_;
public:
	GrpDounut() :
		div_(10)
	{}

	void radius(const Vec2<float>& radius) {
		radius_ = radius;
	}
	void radius(const float x, const float y) {
		radius_.set(x, y);
	}
	void hole(const Vec2<float>& hole) {
		hole_ = hole;
	}
	void hole(const float x, const float y) {
		hole_.set(x, y);
	}

	void div(int div) {
		div_ = div;
	}
		
	virtual void draw() {
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		this->set_mtx();
		
		std::vector<GLfloat> vtx;
		vtx.reserve(div_ * 4);
		for(int i = 0; i <= div_; i += 1)
		{
			float r = (PI * -2.0f * (float)i) / div_;
			float sin_r = sin(r);
			float cos_r = cos(r);

			vtx.push_back(radius_.x * sin_r);
			vtx.push_back(radius_.y * cos_r);
			vtx.push_back(hole_.x * sin_r);
			vtx.push_back(hole_.y * cos_r);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, &vtx[0]);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, vtx.size() / 2);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpSprite : public GrpBase {
	Vec2<float> size_;
	Vec2<float> uv_top_, uv_bottom_;
	bool flip_v_, flip_h_;
	bool filter_;
	TexMng::tex_ptr tex_;
public:
	GrpSprite() :
		flip_v_(),
		flip_h_(),
		filter_(true)
	{}

	void size(const Vec2<float>& size) {
		size_ = size;
	}
	void size(const float x, const float y) {
		size_.set(x, y);
	}

	using GrpBase::center;														// TIPS: 基底クラスのメソッドを取り込む
	void center()
	{
		center_.set((int)size_.x / 2, (int)size_.y / 2);
		// TIPS: 小数点以下は切り捨て
	}

	void texture(const TexMng::tex_ptr tex) {
		tex_ = tex;
	}
	void uv(const Vec2<float>& top, const Vec2<float>& bottom) {
		uv_top_ = top;
		uv_bottom_ = bottom;
	}
	void uv(const Vec2<float>& top) {
		uv_top_ = top;
		uv_bottom_ = top + size_;
	}
	void uv(const float top_x, const float top_y, const float bottom_x, const float bottom_y) {
		uv_top_.set(top_x, top_y);
		uv_bottom_.set(bottom_x, bottom_y);
	}
	void uv(const float top_x, const float top_y) {
		uv_top_.set(top_x, top_y);
		uv_bottom_.set(top_x + size_.x, top_y + size_.y);
	}
	void flip(const bool h, const bool v) {
		flip_h_ = h;
		flip_v_ = v;
	}
	void smooth(const bool smooth) {
		filter_ = smooth;
	}

	virtual void draw()
	{
		glEnable(GL_TEXTURE_2D);
		tex_->bind();
		GLint filter = filter_ ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		GrpSetBlend(blend_);
		this->set_mtx();

		float top_u, top_v;
		float bottom_u, bottom_v;
		const Vec2<int>& size = tex_->size();
		const Vec2<float>& uv = tex_->uv();
			
			
		top_u = uv_top_.x * uv.x / size.x;
		top_v = uv_top_.y * uv.y / size.y;
		bottom_u = uv_bottom_.x * uv.x / size.x;
		bottom_v = uv_bottom_.y * uv.y / size.y;
		if(flip_h_)
		{
			float a = top_u;
			top_u = bottom_u;
			bottom_u = a;
		}
		if(flip_v_)
		{
			float a = top_v;
			top_v = bottom_v;
			bottom_v = a;
		}

		GLfloat vtx[] = {
			0.0f, 0.0f,
			0.0f, size_.y,
			size_.x, 0.0f,
			size_.x, size_.y,
		};
		GLfloat uv_vtx[] = {
			top_u, top_v,
			top_u, bottom_v,
			bottom_u, top_v,
			bottom_u, bottom_v,
		};
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glTexCoordPointer(2, GL_FLOAT, 0, uv_vtx);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpBezierLine : public GrpBase {
	Bezier bx_, by_;
	float div_;
	bool smooth_;
	float width_;
public:
	GrpBezierLine() :
		div_(5.0f),
		smooth_(),
		width_(1.0f)
	{}
		
	void smooth(const bool smooth) {
		smooth_ = smooth;
	}
	void width(const float width) {
		width_ = width;
	}
	void div(float div) {
		div_ = div;
	};
	void set(float x0, float y0, float x1, float y1, float x_p1, float y_p1, float x_p2, float y_p2) {
		pos_.set(x0, y0);
		bx_.set(0.0f, x1 - x0, x_p1 - x0, x_p2 - x0);
		by_.set(0.0f, y1 - y0, y_p1 - y0, y_p2 - y0);
	}
	void set(Vec2<float>& st, Vec2<float>& ed, Vec2<float>& p1, Vec2<float>& p2) {
		pos_ = st;
		bx_.set(0.0f, ed.x - st.x, p1.x - st.x, p2.x - st.x);
		by_.set(0.0f, ed.y - st.y, p1.y - st.y, p2.y - st.y);
	}

	virtual void draw() {
		glDisable(GL_TEXTURE_2D);
		GrpSetBlend(blend_);
		glLineWidth(width_);
		if(smooth_) glEnable(GL_LINE_SMOOTH);
		else		    glDisable(GL_LINE_SMOOTH);
		this->set_mtx();

		float len = BezierLength(bx_, by_, 8);
		float dt = div_ / len;
		if(dt > 1.0f) dt = 1.0f;
		float t = 0.0f;

		std::vector<GLfloat> vtx;
		bool done = false;
		while(1)
		{
			float x0 = bx_.pos(t);
			float x1 = bx_.pos(t + dt);
			float y0 = by_.pos(t);
			float y1 = by_.pos(t + dt);

			vtx.push_back(x0);
			vtx.push_back(y0);
			vtx.push_back(x1);
			vtx.push_back(y1);
				
			t += dt;
			if(t > 1.0)
			{
				if(done) break;
				done = true;
				t = 1.0;
			}
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, &vtx[0]);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glDrawArrays(GL_LINE_STRIP, 0, vtx.size() / 2);
		// glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
};

class GrpRoundBox : public GrpBase {
	Vec2<float> size_;
	bool filter_;
	TexMng::tex_ptr tex_;
public:
	GrpRoundBox() :
		filter_(true)
	{}

	void size(const Vec2<float>& size) {
		size_ = size;
	}
	void size(const float x, const float y) {
		size_.set(x, y);
	}

	using GrpBase::center;														// TIPS: 基底クラスのメソッドを取り込む
	void center()
	{
		center_.set((int)size_.x / 2, (int)size_.y / 2);
		// TIPS: 小数点以下は切り捨て
	}

	void texture(const TexMng::tex_ptr tex) {
		tex_ = tex;
	}
	void smooth(const bool smooth) {
		filter_ = smooth;
	}

	virtual void draw()
	{
		glEnable(GL_TEXTURE_2D);
		tex_->bind();
		GLint filter = filter_ ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		GrpSetBlend(blend_);
		this->set_mtx();

		const Vec2<int>& size = tex_->size();
		float bottom_u = size_.x / (size.x * 2.0f);
		float bottom_v = size_.y / (size.y * 2.0f);

		GLfloat vtx[] = {
			size_.x / 2.0f, size_.y / 2.0f,
			size_.x / 2.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, size_.y / 2.0f,
			0.0f, size_.y,
			size_.x / 2.0f, size_.y,
			size_.x, size_.y,
			size_.x, size_.y / 2.0f,
			size_.x, 0.0f,
			size_.x / 2.0f, 0.0f,
		};
		GLfloat uv_vtx[] = {
			bottom_u, bottom_v,
			bottom_u, 0.0,
			0.0, 0.0,
			0.0, bottom_v,
			0.0, 0.0,
			bottom_u, 0.0,
			0.0, 0.0,
			0.0, bottom_v,
			0.0, 0.0,
			bottom_u, 0.0,
		};
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(col_.r, col_.g, col_.b, col_.a);
		glTexCoordPointer(2, GL_FLOAT, 0, uv_vtx);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 10);

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPopMatrix();
	}
};

}
