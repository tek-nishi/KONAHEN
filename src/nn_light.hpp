
#pragma once

#include <iostream>
#include "co_vec3.hpp"

namespace ngs {

class Light
{
	GLenum hdl_;
	GLfloat scale_;
	GLfloat amb_[4];
	GLfloat diff_[4];
	GLfloat pos_[4];
	Vec3<float> ofs_;
	GLfloat constant_;
	bool spot_;
	GLfloat cutoff_;
	GLfloat exponent_;
	GLfloat base_;
	
public:
	Light() :
		hdl_(GL_LIGHT0),
		scale_(1.0),
		constant_(1.0),
		spot_(),
		cutoff_(90),
		exponent_(),
		base_(1)
	{
		amb_[0] = amb_[1] = amb_[2] = 0.0;
		amb_[3] = 1.0;

		diff_[0] = diff_[1] = diff_[2] = 0.0;
		diff_[3] = 1.0;

		pos_[0] = pos_[1] = pos_[2] = 0.0;
		pos_[3] = 1.0;
	}
	~Light() {}

	void handle(const GLenum hdl)
	{
		hdl_ = hdl;
	}
	void scale(const float scale)
	{
		scale_ = scale;
	}
	float getScale() const { return scale_; }
	
	void ambientCol(const float red, const float green, const float blue)
	{
		amb_[0] = red;
		amb_[1] = green;
		amb_[2] = blue;
	}
	
	void diffuseCol(const float red, const float green, const float blue)
	{
		diff_[0] = red;
		diff_[1] = green;
		diff_[2] = blue;
	}
	
	void pos(const Vec3<float>& pos)
	{
		pos_[0] = pos.x;
		pos_[1] = pos.y;
		pos_[2] = pos.z;
	}

	void ofs(const Vec3<float>& ofs)
	{
		ofs_ = ofs;
	}
	const Vec3<float>& getOfs() const { return ofs_; }

	void constant(const float constant)
	{
		constant_ = constant;
	}

	void spot(const bool spot) { spot_ = spot; }
	void cutoff(const GLfloat cutoff) { cutoff_ = cutoff; }
	void exponent(const GLfloat exponent) { exponent_ = exponent; }
	void base(const GLfloat base) { base_ = base; }

	void setup()
	{
		glEnable(GL_LIGHTING);
		glEnable(hdl_);

		GLfloat amb[4] = {
			amb_[0] * base_ * scale_,
			amb_[1] * base_ * scale_,
			amb_[2] * base_ * scale_,
			amb_[3] * base_ * scale_
		};
		glLightfv(hdl_, GL_AMBIENT, amb);

		GLfloat diff[4] = {
			diff_[0] * base_ * scale_,
			diff_[1] * base_ * scale_,
			diff_[2] * base_ * scale_,
			diff_[3] * base_ * scale_
		};
		glLightfv(hdl_, GL_DIFFUSE, diff);

		glLightfv(hdl_, GL_POSITION, pos_);

		if (spot_)
		{
			glLightf(hdl_, GL_SPOT_CUTOFF,  cutoff_);
			glLightf(hdl_, GL_SPOT_EXPONENT, exponent_);

			Vec3<GLfloat> v(-pos_[0], -pos_[1], -pos_[2]);
			v.unit();
			GLfloat spot[3] = {
				v.x, v.y, v.z
			};
			glLightfv(hdl_, GL_SPOT_DIRECTION, spot);
			// FIXME:↑事前に計算しておく
		}

		glLightf(hdl_, GL_CONSTANT_ATTENUATION, constant_);
	}

};

}
