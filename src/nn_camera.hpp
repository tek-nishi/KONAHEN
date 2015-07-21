
#pragma once

#include <cmath>
#include <iostream>
#include "co_vec3.hpp"
#include "co_quat.hpp"
#include "nn_misc.hpp"

namespace ngs {

class Camera {
	int mode_;
	bool oblong_;																			// 縦長

	float angle_;
	float aspect_;
	float nearZ_, farZ_;
	Vec3<float> at_;
	Vec3<float> pos_;
	Quat<float> rot_;
	float dist_;
	float width_, height_;
	float pitch_max_;

	GLdouble proj_[16];
	GLdouble model_[16];
	GLint view_[4];

	Matrix rotMtx_;
	
	void settings()
	{
		if (oblong_)
		{
			width_ = nearZ_ * tan(angle_ / 2.0);
			height_ = width_ / aspect_;
		}
		else
		{
			height_ = nearZ_ * tan(angle_ / 2.0);
			width_ = height_ * aspect_;
		}
	}

public:
	enum {
		PERSPECTIVE,
		ORTHOGONAL,
	};

	explicit Camera(const int mode) :
		mode_(mode),
		oblong_(),
		angle_(30.0 * PI / 180.0),
		aspect_(1.0),
		nearZ_(1.0),
		farZ_(10.0),
		dist_(1.0),
		width_(640.0 / 2.0),
		height_(320.0 / 2.0),
		pitch_max_(PI)
	{}

	~Camera() {}

	void setAngle(const float angle)
	{
		angle_ = angle;
		this->settings();
	}
	float getAngle() const
	{
		return angle_;
	}

	void oblong(const bool oblong) { oblong_ = oblong; }
	bool oblong() const { return oblong_; }

	void setAspect(const float aspect)
	{
		aspect_ = aspect;
		this->settings();
	}
	float getAspect() const
	{
		return aspect_;
	}

	const float width() const { return width_; }
	const float height() const { return height_; }
	
	void setSize(const float width, const float height)
	{
		width_ = width;
		height_ = height;
	}
	Vec2<float> getSize() const
	{
		return Vec2<float>(width_ * 2.0, height_ * 2.0);
	}

	void setNearZ(const float z)
	{
		nearZ_ = z;
		this->settings();
	}
	float getNearZ() const
	{
		return nearZ_;
	}
	
	void scale(const float sx, const float sy)
	{
		float w = width_ * sx;
		float h = height_ * sy;

		aspect_ = w / h;
		angle_ = atan((oblong_ ? w : h) / nearZ_) * 2.0;
		this->settings();
	}

	void setFarZ(const float z)
	{
		farZ_ = z;
	}
	float getFarZ() const
	{
		return farZ_;
	}

	void setPos(const Vec3<float>& pos)
	{
		pos_ = pos;
	}
	const Vec3<float>& getPos() const
	{
		return pos_;
	}

	void setRot(const Quat<float>& rot)
	{
		rot_ = rot;
	}
	const Quat<float>& getRot() const
	{
		return rot_;
	}

	void setDist(const float dist)
	{
		dist_ = dist;
	}
	float getDist() const
	{
		return dist_;
	}

	void setMode(const int mode)
	{
		mode_ = mode;
	}
	int getMode() const
	{
		return mode_;
	}

	const Matrix& getMatrix() const
	{
		return rotMtx_;
	}

	const GLdouble *model() const {
		return model_;
	}

	void pitch_max(const float pitch_max) { pitch_max_ = pitch_max; }
	float pitch_max() const { return pitch_max_; }
	
	void setup()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		switch(mode_)
		{
		case PERSPECTIVE:
			{
				glFrustum(width_, -width_, -height_, height_, nearZ_, farZ_);

				glMatrixMode(GL_MODELVIEW);
				// TIPS:カメラの行列はMODELVIEWに格納しないとFOGなどが正しく処理されない
				glTranslated(0.0, 0.0, -dist_);
				rotMtx_.rotate(rot_);
				glMultMatrixd(rotMtx_.value());
				rotMtx_.reverse(rotMtx_);
			}
			break;

			case ORTHOGONAL:
			{
				glOrtho(-width_ / 2.0, width_ / 2.0, height_ / 2.0, -height_ / 2.0, -1.0, 1.0);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
			}
			break;
		}

		glGetDoublev(GL_PROJECTION_MATRIX, proj_);
		glGetDoublev(GL_MODELVIEW_MATRIX, model_);
		glGetIntegerv(GL_VIEWPORT, view_);
	}

	Vec3<float> posToWorld(const Vec2<float>& pos, const float z, const GLdouble *model) const
	{
		GLdouble ox, oy, oz;

		gluUnProject(pos.x, pos.y, z, model, proj_, view_, &ox, &oy, &oz);
		return Vec3<float>(ox, oy, oz);
	}

	Vec3<float> posToScreen(const Vec3<float>& pos, const GLdouble *model) const
	{
		GLdouble sx, sy, sz;
		gluProject(pos.x, pos.y, pos.z, model, proj_, view_, &sx, &sy, &sz);
		return Vec3<float>(sx, sy, sz);
	}
	
};

// スクリーン上のposと球体の交差を調べてhit_posに格納する
bool crossSpherePos(Vec3<float>& hit_pos, const Vec2<float>& pos, const float radius, const Vec3<float>& center, const Camera& camera, const GLdouble *mtx)
{
	Vec3<float> pos_near = camera.posToWorld(pos, 0.0, mtx);
	Vec3<float> pos_far = camera.posToWorld(pos, 1.0, mtx);
	Vec3<float> v = pos_far - pos_near;
	v.unit();
	return sphereRayCollision(hit_pos, radius, center, pos_near, v);
}

}
