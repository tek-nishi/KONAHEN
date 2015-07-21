
#pragma once

//
// 全問題に対する正解率の表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

class CorrectRate : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	float delay_;
	
	Vec2<float> ofs_;
	Vec2<float> ofs_st_;
	Vec2<float> ofs_ed_;
	float time_cur_;
	float time_ed_;

	Vec2<float> num_ofs_;

	WidgetsMap::WidgetPtr widget_;
	const TexMng::tex_ptr texture_;
	
	float rate_;
	float rate_ed_;
	float rate_time_, rate_time_ed_;

	bool exec_fin_;

public:
	explicit CorrectRate(GameEnv& env, const bool review) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		delay_(params_["correct_delay"].get<double>()),
		ofs_st_(params_["correct_ofs_x"].get<double>(), params_["correct_ofs_y"].get<double>()),
		time_cur_(),
		time_ed_(params_["correct_ease"].get<double>()),
		num_ofs_(params_["correct_num_x"].get<double>(), params_["correct_num_y"].get<double>()),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		rate_(),
		rate_ed_(PlaceCorrectRate(env) * 100.0f),
		rate_time_(),
		rate_time_ed_(params_["correct_time"].get<double>()),
		exec_fin_()
	{
		DOUT << "CorrectRate()" << std::endl;

		const WidgetsMap widgets(params_["correct_widgets"].get<picojson::object>(), env_.texMng, *env_.path, env_.size, env_.y_bottom);
		widget_ = widgets.get("correct");
		if (review)
		{
			ofs_st_.y = params_["review_y"].get<double>();
			ofs_ed_.y = ofs_st_.y;
			delay_ = 0.0f;
			// 復習モード
		}
	}
	
	~CorrectRate()
	{
		DOUT << "~CorrectRate()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if ((delay_ > 0.0) && ((delay_ -= delta_time) > 0.0)) return;
		
		Easing easing;
		if (time_cur_ < time_ed_)
		{
			time_cur_ += delta_time;
			easing.ease(ofs_, time_cur_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);
		}

		if (rate_time_ < rate_time_ed_)
		{
			rate_time_ += delta_time;
			easing.ease(rate_, rate_time_, 0.0f, rate_ed_, rate_time_ed_, QUART_OUT);
		}
		
		if (exec_fin_ && (time_cur_ >= time_ed_)) active_ = false;
	}

	void draw()
	{
		if (delay_ > 0.0) return;
		
		GrpCol<float> col(1, 1, 1, 1);
		{
			const Vec2<float>& pos = widget_->posOrig();
			widget_->setPos(pos + ofs_);
			widget_->setCol(col);
			widget_->draw();
		}

		{
			Vec2<float> pos = widget_->dispPos();
			pos += num_ofs_;
			DrawNumberSmall(static_cast<int>(rate_), 3, pos, col, texture_);
			pos.x += 22 * 3 - 5;
			DrawNumberSmall(10, 1, pos, col, texture_);
			pos.x += 22 - 5;
			DrawNumberSmall(static_cast<int>(rate_ * 10.0) % 10, 1, pos, col, texture_);
			pos.x += 22;
			DrawNumberSmall(13, 1, pos, col, texture_);
		}
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
				delay_ = 0;
				time_cur_ = 100;
				ofs_ = ofs_ed_;
			}
			break;

		case MSG_TITLE_CREDITS_PUSH:
		case MSG_TITLE_GAMESTART:
		case MSG_TITLE_END:
			if (!exec_fin_)
			{
				exec_fin_ = true;
				time_cur_ = 0;
				ofs_st_ = ofs_;
				ofs_ed_.set(params_["correct_ofs_x"].get<double>(), params_["correct_ofs_y"].get<double>());
			}
			break;
		}
	}
	
};


}
