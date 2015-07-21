
#pragma once

//
// 残り時間表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {


class PlayTime : public TaskProc
{
	GameEnv& env_;
	bool active_;
	
	picojson::object& param_;

	Vec2<float> ofs_;
	Vec2<float> ofs_st_;
	Vec2<float> ofs_ed_;
	float time_cur_;
	float time_ed_;

	WidgetsMap::WidgetPtr widget_;
	const TexMng::tex_ptr texture_;

	Vec2<float> num_ofs_;
	
	GrpCol<float> col_;
	GrpCol<float> col_num_;
	float alpha_r_;
	bool alpha_exec_;

	float time_;
	
	bool exec_start_;
	bool exec_fin_;

public:
	PlayTime(GameEnv& env, const WidgetsMap& widgets) :
		env_(env),
		active_(true),
		param_(env.params->value().get<picojson::object>()["playtime"].get<picojson::object>()),
		ofs_st_(-220, 0),
		time_cur_(),
		time_ed_(1),
		widget_(widgets.get("timeremain")),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		col_(1, 1, 1, 1),
		col_num_(1, 1, 1, 1),
		alpha_r_(),
		alpha_exec_(true),
		time_(),
		exec_start_(true),
		exec_fin_()
	{
		DOUT << "PlayTime()" << std::endl;

		picojson::array& array = param_["num_ofs"].get<picojson::array>();
		num_ofs_.x = array[0].get<double>();
		num_ofs_.y = array[1].get<double>();
	}
	~PlayTime()
	{
		DOUT << "~PlayTime()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (time_cur_ < time_ed_) time_cur_ += delta_time;

		Easing easing;
		easing.ease(ofs_, time_cur_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);

		if (exec_start_)
		{
			easing.ease(time_, time_cur_, 0, env_.time, time_ed_, CUBIC_OUT);
			exec_start_ = (time_cur_ < time_ed_);
		}
		else
		{
			time_ = env_.time;
		}
		
		if (env_.started && env_.time < 10.0 && alpha_exec_)
		{
			col_.set(1, 0, 0, 1);
			alpha_r_ += delta_time;
			float alpha;
			easing.ease(alpha, alpha_r_, 1.0, 0.5, 1.0, SINE_IN);
			col_num_.set(alpha, 0, 0, 1);
			if (alpha_r_ > 1.0)
			{
				alpha_r_ -= 1;
			}
		}
		
		if (exec_fin_ && (time_cur_ >= time_ed_)) active_ = false;
	}

	void draw()
	{
		const Vec2<float>& pos = widget_->posOrig();
		widget_->setPos(pos + ofs_);
		
		// if (env_.started && env_.time < 4.0) col.set(1, 0, 0, 1);
		widget_->setCol(col_);
		widget_->draw();

		{
			Vec2<float> pos = widget_->dispPos();
			pos += num_ofs_;
			DrawNumberBig(static_cast<int>(time_), 2, pos, col_num_, texture_);

			pos.x += 42 * 1 + 30;
			DrawNumberBig(10, 1, pos, col_num_, texture_);

			pos.x += 34;
			pos.y += 28;
			DrawNumberSmall(static_cast<int>(time_ * 100), 2, pos, col_num_, texture_);
		}
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_END:
			{
				alpha_exec_ = false;
				col_num_ = col_;
			}
			break;
			
		case MSG_GAME_CLEANUP:
			{
				active_ = false;
			}
			break;

		case MSG_GAME_MENU_OFF:
			{
				time_cur_ = 0.0f;
				time_ed_ = 1.5;
				ofs_st_.set(0, 0);
				ofs_ed_.set(-220, 0);
				exec_fin_ = true;
			}
			break;

		case MSG_GAME_LEVELUP:
			if (env_.time >= 0.0 && !(env_.time < 10.0))
			{
				col_.set(1, 1, 1, 1);
				col_num_ = col_;
			}
			break;
		}
	}
	
};


}
