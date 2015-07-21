
#pragma once

//
// 一回で「こなへん」できた時の演出
//

#include <iostream>
#include <string>
#include "co_texmng.hpp"
#include "co_graph.hpp"
#include "co_easearray.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

struct TouchJustParam {
	TexMng *const			 texMng;
	int								 just;
	picojson::object&	 params;
	const std::string& path;
};


class TouchJust : public TaskProc
{
	bool active_;

	picojson::object& params_;

	WidgetsMap::WidgetPtr widget_;
	WidgetsMap::WidgetPtr widget2_;
	const TexMng::tex_ptr texture_;
	
	EaseArray<Vec2<float> > easeArray_;
	float time_;

	int just_;
	Vec2<float> ofs_;
	float alpha_;
	Vec2<float> num_ofs_;
	
public:
	TouchJust(const TouchJustParam& param, const WidgetsMap& widgets) :
		active_(true),
		params_(param.params["just"].get<picojson::object>()),
		widget_(widgets.get("just")),
		widget2_(widgets.get("just_x")),
		texture_(param.texMng->read(param.path + "devdata/game.png")),
		time_(),
		just_(param.just)
	{
		DOUT << "TouchJust()" << std::endl;
		
		EasingArayVec2(EXPO_OUT, params_["easing"].get<picojson::array>(), easeArray_);

		picojson::array& array = params_["num_ofs"].get<picojson::array>();
		num_ofs_.x = array[0].get<double>();
		num_ofs_.y = array[1].get<double>();
	}
	
	~TouchJust() {
		DOUT << "~TouchJust()" << std::endl;
	}

	bool active() const { return active_; }

	void step(const float delta_time)
	{
		time_ += delta_time;

		Vec2<float> res; 
		bool ease_fin = easeArray_.ease(res, time_);
		ofs_.x = res.x;
		alpha_ = res.y;
		if (!ease_fin) active_ = false;
	}

	void draw()
	{
		GrpCol<float> col(1, 1, 1, alpha_);
		{
			const Vec2<float>& pos = widget_->posOrig();
			widget_->setPos(pos + ofs_);
			widget_->setCol(col);
			widget_->draw();
		}

		if(just_ > 1)
		{
			{
				const Vec2<float>& pos = widget2_->posOrig();
				widget2_->setPos(pos + ofs_);
				widget2_->setCol(col);
				widget2_->draw();
			}

			{
				Vec2<float> pos = widget2_->dispPos();
				pos += num_ofs_;

				int num = (just_ > 9) ? 2 : 1;
				int val = (just_ > 9) ? just_ : just_ % 10;
				DrawNumberSmall(val, num, pos, col, texture_);
			}
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
		}
	}
};

}
