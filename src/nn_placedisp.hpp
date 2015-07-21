
#pragma once

//
// 場所名表示
//

#include <string>
#include "co_fntmng.hpp"
#include "co_task.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class PlaceDisp : public TaskProc {
	GameEnv& env_;
	bool active_;

	picojson::object& param_;

	const TexMng::tex_ptr texture_;
	FntMng::FontPtr font_;

	const std::string& cur_place_;
	const bool& answer_;
	const std::string& ans_place_;

	std::string cur_place_l_;
	std::string ans_place_l_;
	
	bool disp_;
	bool pause_;
	float count_;

	float scale_;
	float scale_st_, scale_ed_;
	float alpha_;
	float time_cur_;
	float time_ed_;

	bool ans_disp_;
	float ans_scale_;
	float ans_scale_st_, ans_scale_ed_;
	float ans_alpha_;
	float ans_time_;
	float ans_time_ed_;

	float correct_disp_, miss_disp_;

	bool ofs_exec_;
	Vec2<float> ofs_;
	Vec2<float> ofs_st_;
	Vec2<float> ofs_ed_;
	float ofs_time_cur_;
	float ofs_time_ed_;

	bool fade_;
	float fade_time_;
	float fade_alpha_;
	float fade_alpha_st_;
	float fade_alpha_ed_;

	bool exec_fin_;
	
public:
	explicit PlaceDisp(GameEnv& env) :
		env_(env),
		active_(true),
		param_(env.params->value().get<picojson::object>()["placedisp"].get<picojson::object>()),
		texture_(env.texMng->read(*env.path + "devdata/round.png")),
		font_(ReadFont("place", *env.fonts, *env.path, env.params->value().get<picojson::object>())),
		cur_place_(env.cur_place),
		answer_(env.answer),
		ans_place_(env.ans_place),
		disp_(),
		pause_(),
		count_(),
		scale_(),
		scale_st_(param_["scale_st"].get<double>()),
		scale_ed_(1.0),
		alpha_(),
		time_cur_(),
		time_ed_(param_["time_ed"].get<double>()),
		ans_disp_(),
		ans_scale_(),
		ans_scale_st_(param_["ans_scale_st"].get<double>()),
		ans_scale_ed_(1.0),
		ans_alpha_(),
		ans_time_(),
		ans_time_ed_(param_["ans_time_ed"].get<double>()),
		correct_disp_(param_["correct_disp"].get<double>()),
		miss_disp_(param_["miss_disp"].get<double>()),
		ofs_exec_(),
		fade_(),
		fade_alpha_(1),
		exec_fin_()
	{
		DOUT << "PlaceDisp()" << std::endl;
	}
	~PlaceDisp()
	{
		DOUT << "~PlaceDisp()" << std::endl;
	}

	bool active() const { return active_; }
	
	void step(const float delta_time)
	{
		if (!disp_) return;
		
		if (time_cur_ < time_ed_)
		{
			time_cur_ += delta_time;

			Easing easing;
			easing.ease(scale_, time_cur_, scale_st_, scale_ed_, time_ed_, QUART_OUT);
			easing.ease(alpha_, time_cur_, 0, 1, time_ed_, QUART_OUT);
		}

		if (ans_disp_)
		{
			if (ans_time_ < ans_time_ed_)
			{
				ans_time_ += delta_time;
				Easing easing;
				easing.ease(ans_scale_, ans_time_, ans_scale_st_, ans_scale_ed_, ans_time_ed_, QUART_OUT);
				easing.ease(ans_alpha_, ans_time_, 0, 1, ans_time_ed_, QUART_OUT);
			}
		}

		if (count_ > 0.0 && ((count_ -= delta_time) <= 0.0))
		{
			disp_ = false;
			ans_disp_ = false;
		}
		
		if (ofs_exec_)
		{
			if (ofs_time_cur_ < ofs_time_ed_) ofs_time_cur_ += delta_time;

			Easing easing;
			easing.ease(ofs_, ofs_time_cur_, ofs_st_, ofs_ed_, ofs_time_ed_, QUART_OUT);
			if (exec_fin_ && (ofs_time_cur_ >= ofs_time_ed_)) active_ = false;
		}

		if (fade_)
		{
			fade_time_ += delta_time;

			Easing easing;
			easing.ease(fade_alpha_, fade_time_, fade_alpha_st_, fade_alpha_ed_, 0.4);
			if (fade_time_ >= 0.4) fade_ = false;
		}
	}

	void draw()
	{
		if (!disp_) return;

		glPushMatrix();
#if (TARGET_OS_IPHONE)
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
#endif

		glTranslatef(0, 25, 0);

		float scale = 1.0;
		const Vec2<float>& size_org = font_->size(cur_place_l_);
		Vec2<float> size = size_org;
		if (size.x > (env_.size->x - 40))
		{
			scale = (env_.size->x - 40) / size.x;
			size.x = env_.size->x - 40;
		}

		const Vec2<float>& ans_size_org = answer_ ? font_->size(ans_place_l_) : Vec2<float>(0, 0);
		Vec2<float> ans_size = ans_size_org;
		Vec2<float> bg_size = size;
		float ans_scale = 1.0;
		if (answer_)
		{
			if (ans_size.x > (env_.size->x - 40))
			{
				ans_scale = (env_.size->x - 40) / ans_size.x;
				ans_size.x = env_.size->x - 40;
			}
			
			if (bg_size.x < ans_size.x) bg_size.x = ans_size.x;
			bg_size.y += ans_size.y + 10.0;
		}

		GrpRoundBox obj;
		obj.pos(ofs_.x, env_.size->y / -2.0 + size.y + ofs_.y);
		obj.size(bg_size.x + 30, bg_size.y + 20);
		obj.center();
		obj.texture(texture_);
		obj.col(0, 0, 0, 0.5 * fade_alpha_);
		obj.draw();
			
		font_->pos(-4.0 + ofs_.x, 16.0 + size.y - bg_size.y / 2 + env_.size->y / -2.0 + ofs_.y);
		font_->scale(scale_ * scale, scale_ * scale);
		font_->center(size_org.x / 2, -size_org.y / 2);
		font_->col(1, 1, 1, alpha_ * fade_alpha_);
		font_->draw(cur_place_l_);
		if (answer_ && ans_disp_)
		{
			font_->pos(-4.0 + ofs_.x, 16.0 + bg_size.y - bg_size.y / 2 + env_.size->y / -2.0 + ofs_.y);
			font_->scale(ans_scale_ * ans_scale, ans_scale_ * ans_scale);
			font_->center(ans_size_org.x / 2, -ans_size_org.y / 2);
			font_->col(1, 1, 1, ans_alpha_ * fade_alpha_);
			font_->draw(ans_place_l_);
		}
#if (TARGET_OS_IPHONE)
		glPopClientAttrib();
#endif
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
			
		case MSG_GAME_PLACEDISP_START:
			{
				disp_ = true;
				count_ = 0;
				time_cur_ = 0.0;
				scale_ = scale_st_;
				alpha_ = 0.0;
				ans_disp_ = false;

				cur_place_l_ = env_.localize->get(cur_place_);
				if (answer_)
				{
					ans_place_l_ = env_.localize->get(ans_place_);
					if (env_.onetime)
					{
						ans_place_l_ = "(" + ans_place_l_ + ")";
						// 同時表示の場合は括弧を追加
					}
				}
			}
			break;

		case MSG_GAME_PLACEDISP_ANS:
		case MSG_GAME_TOUCH_ANS:
		case MSG_GAME_TOUCH_IN:
			{
				switch (msg)
				{
				case MSG_GAME_TOUCH_IN:
					count_  = correct_disp_;
					break;

				case MSG_GAME_TOUCH_ANS:
					count_ = miss_disp_;
					break;
					
				}

				if (!ans_disp_)
				{
					ans_disp_ = true;
					ans_time_ = 0.0;
					ans_scale_ = ans_scale_st_;
					ans_alpha_ = 0.0;
				}
			}
			break;

		case MSG_GAME_PAUSE:
			{
				pause_ = !pause_;

				fade_ = true;
				fade_time_ = 0.0;
				fade_alpha_st_ = fade_alpha_;
				fade_alpha_ed_ = pause_ ? 0.0 : 1.0;
			}
			break;

		case MSG_GAME_MENU_OFF:
			{
				ofs_time_cur_ = 0.0f;
				ofs_time_ed_ = 1.5;
				ofs_st_.set(0, 0);
				ofs_ed_.set(0, -200);
				ofs_exec_ = true;
				exec_fin_ = true;
			}
			break;
		}
	}
};

}
