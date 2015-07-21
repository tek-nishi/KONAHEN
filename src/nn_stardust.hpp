
#pragma once

//
// 背景の星屑を表示
//

#include "co_vec3.hpp"
#include "co_matrix.hpp"
#include "co_easearray.hpp"
#include "nn_camera.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class StarDust : public TaskProc {
	GameEnv& env_;
	bool active_;
	
	picojson::object& params_;
	const u_int num_;
	const GLfloat speed_;

	EaseArray<float> easeArray_;
	float alpha_;
	float alpha_time_;

	struct Vtx {
		GLfloat x, y, z;
	};
	struct Color {
		GLfloat r, g, b, a;
	};
	struct Body {
		Vtx vtx;
		Color col;
	};
	std::vector<Body> body_;
	// OpenGLのストライドに対応する為、少々回りくどい定義になっている
	
public:
	StarDust(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()),
		num_(params_["stardust_num"].get<double>()),
		speed_(params_["stardust_speed"].get<double>()),
		alpha_(),
		alpha_time_()
	{
		DOUT << "StarDust()" << std::endl;

		const picojson::array& array = params_["star_col"].get<picojson::array>();
		std::vector<GrpCol<float> > colTbl;
		for (picojson::array::const_iterator it = array.begin(); it != array.end(); ++it)
		{
			const picojson::array& col = it->get<picojson::array>();
			GrpCol<float> c(col[0].get<double>(), col[1].get<double>(), col[2].get<double>(), 1);
			colTbl.push_back(c);
			// 色データを読み込んでおく
		}

		{
			const picojson::array& array = params_["stardust_easing"].get<picojson::array>();
			EasingArayFloat(CUBIC_INOUT, array, easeArray_);
			// アルファのイージング
		}

		body_.reserve(64);
		for (u_int i = 0; i < num_; ++i)
		{
			float x = ((float)(rand() % 500) / 500.0f) * 20000.0f - 10000.0f;
			float y = ((float)(rand() % 500) / 500.0f) * 20000.0f - 10000.0f;
			float z = ((float)(rand() % 500) / 500.0f) * -10000.0f;
			const GrpCol<float>& col = colTbl[rand() % colTbl.size()];

			Body b = {
				{ x, y, z },
				{ col.r, col.g, col.b, col.a }
			};
			body_.push_back(b);
		}
	}

	~StarDust()
	{
		DOUT << "~StarDust()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		alpha_time_ += delta_time;
		bool res = easeArray_.ease(alpha_, alpha_time_);
		if (!res) active_ = false;
		
		for (std::vector<Body>::iterator it = body_.begin(); it != body_.end(); ++it)
		{
			it->vtx.z += speed_ * delta_time;
			if (it->vtx.z > 0.0f) it->vtx.z -= 10000.0f;
			it->col.a = alpha_;
		}
	}
	
	void draw()
	{
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		GrpSetBlend(GRP_BLEND_ADD);
		glDisable(GL_POINT_SMOOTH);

		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		static const GLfloat f_col[] = { 0, 0, 0, 1 };
		glFogfv(GL_FOG_COLOR, f_col);
		float z = env_.camera->getFarZ();
		glFogf(GL_FOG_START, z * 0.8f);
		glFogf(GL_FOG_END, z);

		glPointSize(1.5);

		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glVertexPointer(3, GL_FLOAT, sizeof(Body), &body_[0]);
		glColorPointer(4, GL_FLOAT, sizeof(Body), (GLbyte *)(&body_[0]) + (sizeof(Vtx)));
		glDrawArrays(GL_POINTS, 0, num_);

		glPopClientAttrib();
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

		case MSG_TITLE_INTRO_SKIP:
			{
				active_ = false;
			}
			break;
		}
	}
};

}
