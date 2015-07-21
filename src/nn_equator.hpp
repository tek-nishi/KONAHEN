
#pragma once

//
// 赤道を表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"


namespace ngs {

class Equator : public TaskProc {
	GameEnv& env_;
	bool active_;

	const Camera& camera_;

	picojson::object& params_;
	const float radius_;
	const float scale_;
	const float div_;
	float width_;

	std::vector<GLfloat> vtx_;
	GLuint vbo_;

	float time_, time_ed_;
	float alpha_;
	float alpha_st_, alpha_ed_;

	bool fin_exec_;

public:
	explicit Equator(GameEnv& env) :
		env_(env),
		active_(true),
		camera_(*env.camera),
		params_(env.params->value().get<picojson::object>()["equator"].get<picojson::object>()),
		radius_(env.earth->getRadius()),
		scale_(params_["scale"].get<double>()),
		div_(params_["div"].get<double>()),
		width_(params_["width"].get<double>()),
		time_(),
		time_ed_(2.0),
		alpha_(),
		alpha_st_(),
		alpha_ed_(1.0),
		fin_exec_()
	{
		DOUT << "Equator()" << std::endl;

		if (env_.retina) width_ *= 2.0f;
		
		float radius = radius_ * scale_;
		vtx_.reserve(div_ * 2);
		for(int i = 0; i < div_; ++i)
		{
			float r = (PI * 2.0f * (float)i) / div_;
			vtx_.push_back(radius * sin(r));
			vtx_.push_back(radius * cos(r));

			r += (PI * 2.0f * 0.5f) / div_;
			vtx_.push_back(radius * sin(r));
			vtx_.push_back(radius * cos(r));
		}

		if (use_glex_vbo)
		{
			glGenBuffers(1, &vbo_);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vtx_.size(), &vtx_[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);								// 割り当てを解除しておく
		}
	}

	~Equator()
	{
		DOUT << "~Equator()" << std::endl;

		if (use_glex_vbo)
		{
			glDeleteBuffers(1, &vbo_);
		}
	}
	
	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (time_ < time_ed_)
		{
			time_ += delta_time;
			Easing easing;
			easing.ease(alpha_, time_, alpha_st_, alpha_ed_, time_ed_);

			if (fin_exec_)
			{
				bool active = (time_ < time_ed_);
				if (!active) active_ = false;
			}
		}
	}

	void draw()
	{
		glPushMatrix();
		glLoadMatrixd(&(env_.earth->mtx()[0]));
		// glRotatef(env_.earth->rotate(), 0.0, 1.0, 0.0);
		glRotatef(90, 1.0, 0.0, 0.0);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		GrpSetBlend(GRP_BLEND_ADD);

		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);

		static const GLfloat f_col[] = { 0, 0, 0, 1 };
		glFogfv(GL_FOG_COLOR, f_col);

		float d = camera_.getDist();
		float r = asin((radius_) / d);
		float c_r = cos(r);
		float x = d * c_r;
		float dist = x * c_r;

		float z_dist = 300 + 50 - 50 * (d / camera_.getFarZ());
		glFogf(GL_FOG_START, dist);
		glFogf(GL_FOG_END, dist + z_dist);

		glLineWidth(width_);
		glEnable(GL_LINE_SMOOTH);

		glEnableClientState(GL_VERTEX_ARRAY);
		glColor4f(1, 0, 0, alpha_);

		if (use_glex_vbo)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_);
			glVertexPointer(2, GL_FLOAT, 0, 0);
			glDrawArrays(GL_LINES, 0, vtx_.size() / 2);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		else
		{
			glVertexPointer(2, GL_FLOAT, 0, &vtx_[0]);
			glDrawArrays(GL_LINES, 0, vtx_.size() / 2);
		}

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_FOG);
		glPopMatrix();
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_CLEANUP:
			{
				active_ = false;
			}
			break;

		case MSG_GAME_END:
			if (!fin_exec_)
			{
				fin_exec_ = true;
				time_ = 0.0f;
				alpha_st_ = alpha_;
				alpha_ed_ = 0.0f;
			}
			break;
		}
	}
	
};

}
