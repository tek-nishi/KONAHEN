
#pragma once

//
// 地球にタッチした時の演出
//

#include <iostream>
#include <string>
#include "co_vec3.hpp"
#include "co_quat.hpp"
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class TouchEft : public TaskProc
{
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	const TexMng::tex_ptr texture_;
	const float radius_;
	const float scale_;
	
	bool hit_;
	float time_;
	float time_ed_;
	float alpha_;
	GrpCol<float> col_;

	bool blink_;
	float blink_timer_;
	const float blink_speed_;
	float blink_alpha_;

public:
	explicit TouchEft(GameEnv& env) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["toucheft"].get<picojson::object>()),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		radius_(env.earth->getRadius()),
		scale_(params_["scale"].get<double>()),
		hit_(),
		blink_(),
		blink_timer_(),
		blink_speed_(params_["blink_speed"].get<double>()),
		blink_alpha_(1.0)
	{
		DOUT << "TouchEft()" << std::endl;
	}
	~TouchEft()
	{
		DOUT << "~TouchEft()" << std::endl;
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		if(hit_)
		{
			hit_ = (time_ed_ < 0.0f) || ((time_ += delta_time) < time_ed_);

			if (blink_)
			{
				blink_timer_ += delta_time;
				blink_alpha_ = fabs(sin(PI * 2.0f * (blink_timer_ / blink_speed_)));
				if (blink_timer_ >= blink_speed_) blink_timer_ -= blink_speed_;
			}

			Easing easing;
			easing.ease(alpha_, time_, 1.0, 0.0, time_ed_, QUART_IN);
		}
	}

	void draw()
	{
		if(hit_)
		{
			glPushMatrix();

			glLoadMatrixd(&(env_.earth->mtx()[0]));
			Vec3<float> v1(0, 0, 1);
			Vec3<float> v2(env_.hit_pos.x, env_.hit_pos.y, env_.hit_pos.z);
			v2.unit();
			Vec3<float> cross = v1.cross(v2);
			float r = v1.angle(v2);
			glRotatef(Rad2Ang(r), cross.x, cross.y, cross.z);
			glTranslatef(0, 0, radius_);
			glScalef(scale_, scale_, 1);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glEnable(GL_BLEND);
			glDisable(GL_LIGHTING);

			GrpSprite obj;
			obj.size(10, 10);
			obj.center(5, 5);
			obj.pos(0, 0);
			obj.texture(texture_);
			obj.uv(305, 75, 305 + 57, 75 + 57);
			obj.col(col_.r, col_.g, col_.b, alpha_ * blink_alpha_);
			obj.draw();

			glPopMatrix();
		}
	}

	void msg(const int msg)
	{
		switch(msg)
		{
		case MSG_GAME_CLEANUP:
			{
				hit_ = false;
			}
			break;
			
		case MSG_GAME_TOUCH_IN:
			{
				hit_ = true;
				time_ = 0;
				time_ed_ = params_["touch_in"].get<double>();
				col_.set(1, 0, 0, 1);
			}
			break;

		case MSG_GAME_TOUCH_OUT:
			{
				hit_ = true;
				time_ = 0;
				time_ed_ = params_["touch_out"].get<double>();
				col_.set(1, 1, 1, 1);
			}
			break;

		case MSG_GAME_TOUCH_ANS:
			{
				hit_ = true;
				time_ = 0;
				time_ed_ = params_["touch_ans"].get<double>();
				col_.set(0, 0, 1, 1);
			}
			break;
			
		case MSG_GAME_TOUCH_DISP:
			{
				hit_ = true;
				time_ = 0;
				time_ed_ = -1.0;
				// 不本意ながらマイナスの値で無限に表示
				col_.set(0, 0, 1, 1);
			}
			break;

		case MSG_GAME_TOUCH_BLINK_ON:
			{
				blink_ = true;
				blink_timer_ = 0.0f;
				blink_alpha_ = 0.0f;
			}
			break;

		case MSG_GAME_TOUCH_BLINK_OFF:
			{
				blink_ = false;
				blink_alpha_ = 1.0f;
			}
			break;
		}
	}
};

}
