
#pragma once

//
// こなへん数表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {


class PlayKonahen : public TaskProc {
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
	
	bool exec_fin_;

public:
	PlayKonahen(GameEnv& env, const WidgetsMap& widgets) :
		env_(env),
		active_(true),
		param_(env.params->value().get<picojson::object>()["playkonahen"].get<picojson::object>()),
		ofs_st_(-150, 0),
		time_cur_(),
		time_ed_(1),
		widget_(widgets.get("konahen")),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		exec_fin_()
	{
		DOUT << "PlayKonahen()" << std::endl;

		picojson::array& array = param_["num_ofs"].get<picojson::array>();
		num_ofs_.x = array[0].get<double>();
		num_ofs_.y = array[1].get<double>();
	}
	~PlayKonahen()
	{
		DOUT << "~PlayKonahen()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (time_cur_ < time_ed_) time_cur_ += delta_time;

		Easing easing;
		easing.ease(ofs_, time_cur_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);
		if (exec_fin_ && (time_cur_ >= time_ed_)) active_ = false;
	}

	void draw()
	{
		const Vec2<float>& pos = widget_->posOrig();
		widget_->setPos(pos + ofs_);
		widget_->draw();

		{
			Vec2<float> pos = widget_->dispPos();
			pos += num_ofs_;
			GrpCol<float> col(1, 1, 1, 1);
			DrawNumberBig(env_.konahen, 3, pos, col, texture_);
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

		case MSG_GAME_MENU_OFF:
			{
				time_cur_ = 0.0f;
				time_ed_ = 1.5;
				ofs_st_.set(0, 0);
				ofs_ed_.set(-220, 0);
				exec_fin_ = true;
			}
			break;
		}
	}
	
};


}

