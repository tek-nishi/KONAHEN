
#pragma once

//
// 正解までの距離を表示
//

#include "nn_gameenv.hpp"
#include "nn_misc.hpp"
#include "nn_widgets_map.hpp"


namespace ngs {

float getDistance(const Vec3<float>& pos, const std::vector<Vec3<float> >& center)
{
	float dist = 50000;													// FIXME:地球の円周より大きな値
	for (std::vector<Vec3<float> >::const_iterator it = center.begin(); it != center.end(); ++it)
	{
		float d = pos.angle(*it) * 6378.137f;			// 地球の半径から距離を求める
		if (dist > d) dist = d;
	}
	return dist;
}


class Distance : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& params_;

	Vec2<float> ofs_;
	Vec2<float> ofs_st_;
	Vec2<float> ofs_ed_;
	float time_cur_;
	float time_ed_;

	Vec2<float> num_ofs_;

	WidgetsMap::WidgetPtr widget_;
	WidgetsMap::WidgetPtr widget_km_;
	WidgetsMap::WidgetPtr widget_up_;
	WidgetsMap::WidgetPtr widget_down_;
	const TexMng::tex_ptr texture_;

	bool dist_first_;
	float dist_;
	float dist_st_, dist_ed_;
	float dist_time_, dist_time_ed_;
	bool dist_eft_;

	bool updown_disp_;
	float updown_time_;
	bool updown_near_;
	
	bool exec_fin_;

public:
	Distance(GameEnv& env, const WidgetsMap& widgets) :
		env_(env),
		active_(true),
		params_(env.params->value().get<picojson::object>()["distance"].get<picojson::object>()),
		ofs_st_(params_["ofs_x"].get<double>(), 0),
		time_cur_(),
		time_ed_(0.8),
		num_ofs_(params_["num_x"].get<double>(), params_["num_y"].get<double>()),
		widget_(widgets.get("distance")),
		widget_km_(widgets.get("km")),
		widget_up_(widgets.get("up")),
		widget_down_(widgets.get("down")),
		texture_(env.texMng->read(*env.path + "devdata/game.png")),
		dist_first_(true),
		dist_(),
		dist_st_(),
		dist_ed_(),
		dist_time_(),
		dist_time_ed_(params_["dist_time"].get<double>()),
		dist_eft_(),
		updown_disp_(),
		exec_fin_()
	{
		DOUT << "Distance()" << std::endl;
	}
	
	~Distance()
	{
		DOUT << "~Distance()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (time_cur_ < time_ed_) time_cur_ += delta_time;

		Easing easing;
		easing.ease(ofs_, time_cur_, ofs_st_, ofs_ed_, time_ed_, QUART_OUT);

		if (dist_eft_)
		{
			dist_time_ += delta_time;
			easing.ease(dist_, dist_time_, dist_st_, dist_ed_, dist_time_ed_, QUART_OUT);
			dist_eft_ = dist_time_ < dist_time_ed_;
		}
		
		if (updown_disp_)
		{
			updown_time_ += delta_time;
		}
		
		if (exec_fin_ && (time_cur_ >= time_ed_)) active_ = false;
	}

	void draw()
	{
		GrpCol<float> col(1, 1, 1, 1);
		if (updown_disp_ && !updown_near_)
		{
			col.set(1, 0, 0, 1);
			// 距離が広がったら赤
		}
		
		{
			const Vec2<float>& pos = widget_->posOrig();
			widget_->setPos(pos + ofs_);
			widget_->setCol(1, 1, 1, 1);
			widget_->draw();
		}

		{
			const Vec2<float>& pos = widget_km_->posOrig();
			widget_km_->setPos(pos + ofs_);
			widget_km_->setCol(col);
			widget_km_->draw();
		}

		{
			Vec2<float> pos = widget_km_->dispPos();
			pos += num_ofs_;
			DrawNumberSmall(static_cast<int>(dist_), 5, pos, col, texture_);
		}
		
		if (updown_disp_)
		{
			WidgetsMap::WidgetPtr widget = updown_near_ ? widget_up_ : widget_down_;
      float x = std::abs(sin(updown_time_ * 6.0f) * 8.0f);
			x = updown_near_ ? x : -x;
			Vec2<float> pos = widget->posOrig();
			pos.x += x;
			widget->setPos(pos + ofs_);
			widget->setCol(col);
			widget->draw();
		}
	}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_PLACEDISP_START:
			if (!dist_first_)
			{
				float dist = getDistance(env_.hit_pos, env_.place_center);
				updown_disp_ = false;

				dist_st_ = dist_;
				dist_ed_ = dist;
				dist_time_ = 0;
				dist_eft_ = true;
			}
			break;
			
		case MSG_GAME_TOUCH_IN:
			{
				updown_disp_ = false;
				dist_st_ = dist_;
				dist_ed_ = 0;
				dist_time_ = 0;
				dist_eft_ = true;
				dist_first_ = false;
			}
			break;
			
		case MSG_GAME_TOUCH_OUT:
			{
				float dist = getDistance(env_.hit_pos, env_.place_center);
				updown_disp_ = dist_first_ ? false : true;
				updown_time_ = 0;
				updown_near_ = dist < dist_;
				
				dist_st_ = dist_;
				dist_ed_ = dist;
				dist_time_ = 0;
				dist_eft_ = true;
				dist_first_ = false;
			}
			break;

		case MSG_GAME_TOUCH_ANS:
			{
				dist_first_ = false;
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
				time_ed_ = 1.25;
				ofs_st_.set(0, 0);
				ofs_ed_.set(params_["ofs_x"].get<double>(), 0);
				exec_fin_ = true;
			}
			break;
		}
	}
	
};


}
