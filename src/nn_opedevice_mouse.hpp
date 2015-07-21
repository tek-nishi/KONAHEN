
#if !(TARGET_OS_IPHONE)

#pragma once

//
// DEMO画面での操作表示
// マウス版
//
namespace ngs {

class OpeDevice {
	picojson::object& params_;
	const float ofs_x_, ofs_y_;

	WidgetsMap::WidgetPtr body_;
	WidgetsMap::WidgetPtr cable_;
	WidgetsMap::WidgetPtr btn_l_;
	WidgetsMap::WidgetPtr btn_r_;
	WidgetsMap::WidgetPtr move_l_;
	WidgetsMap::WidgetPtr move_r_;

	bool disp_;

	float ofs_eft_x_;
	float ofs_eft_time_, ofs_eft_time_ed_;

	bool move_;
	float move_alpha_;
	float move_time_, move_time_ed_;
	float move_rot_;
	float move_ofs_;

	bool btn_l_push_;
	Vec3<float> btn_l_col_;
	Vec3<float> btn_l_col_st_, btn_l_col_ed_;
	float btn_l_time_, btn_l_time_ed_;

	bool btn_r_push_;
	Vec3<float> btn_r_col_;
	Vec3<float> btn_r_col_st_, btn_r_col_ed_;
	float btn_r_time_, btn_r_time_ed_;

public:
	explicit OpeDevice(GameEnv& env) :
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		ofs_x_(params_["operate_x"].get<double>()),
		ofs_y_(params_["operate_y"].get<double>()),
		disp_(),
		ofs_eft_time_(),
		ofs_eft_time_ed_(0.8),
		move_(),
		move_alpha_(),
		move_time_ed_(0.25),
		move_rot_(),
		move_ofs_(),
		btn_l_push_(),
		btn_l_col_(1, 1, 1),
		btn_l_col_st_(1, 0, 0),
		btn_l_col_ed_(1, 1, 1),
		btn_l_time_ed_(0.25),
		btn_r_push_(),
		btn_r_col_(1, 1, 1),
		btn_r_col_st_(1, 0, 0),
		btn_r_col_ed_(1, 1, 1),
		btn_r_time_ed_(0.25)
	{
		DOUT << "OpeDevice()" << std::endl; 
		const WidgetsMap widgets(params_["operate_widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		body_ = widgets.get("body");
		cable_ = widgets.get("cable");
		btn_l_ = widgets.get("btn_l");
		btn_r_ = widgets.get("btn_r");
		move_l_ = widgets.get("move_l");
		move_r_ = widgets.get("move_r");
	}

	~OpeDevice()
	{
		DOUT << "~OpeDevice()" << std::endl; 
	}

	void step(const float delta_time)
	{
		Easing easing;
		if (ofs_eft_time_ < ofs_eft_time_ed_)
		{
			ofs_eft_time_ += delta_time;
			easing.ease(ofs_eft_x_, ofs_eft_time_, 150, 0, ofs_eft_time_ed_, QUART_OUT);
			if (ofs_eft_time_ >= ofs_eft_time_ed_) disp_ = true;
		}

		if (move_)
		{
			move_time_ += delta_time;
			easing.ease(move_alpha_, move_time_, 1, 0, move_time_ed_);
			move_rot_ += delta_time;
      move_ofs_ = std::abs(sin(move_rot_ * PI * 1.5f) * 5.0f);
			move_ = move_time_ < move_time_ed_;
		}
		if (btn_l_push_)
		{
			btn_l_time_ += delta_time;
			easing.ease(btn_l_col_, btn_l_time_, btn_l_col_st_, btn_l_col_ed_, btn_l_time_ed_);
			btn_l_push_ = btn_l_time_ < btn_l_time_ed_;
		}
		if (btn_r_push_)
		{
			btn_r_time_ += delta_time;
			easing.ease(btn_r_col_, btn_r_time_, btn_r_col_st_, btn_r_col_ed_, btn_r_time_ed_);
			btn_r_push_ = btn_r_time_ < btn_r_time_ed_;
		}
	}
	void draw()
	{
		glPushMatrix();
		glTranslatef(ofs_x_ + ofs_eft_x_, ofs_y_, 0);
		
		body_->draw();

		btn_l_->setCol(btn_l_col_.x, btn_l_col_.y, btn_l_col_.z, 1);
		btn_l_->draw();
		btn_r_->setCol(btn_r_col_.x, btn_r_col_.y, btn_r_col_.z, 1);
		btn_r_->draw();

		move_l_->setPos(move_ofs_, 0);
		move_l_->setCol(1, 1, 1, move_alpha_);
		move_l_->draw();
		move_r_->setPos(-move_ofs_, 0);
		move_r_->setCol(1, 1, 1, move_alpha_);
		move_r_->draw();

		cable_->draw();

		glPopMatrix();
	}
	
	void touchLeft()
	{
		if (disp_)
		{
			btn_l_push_ = true;
			btn_l_time_ = 0;
			btn_l_time_ed_ = 0.5;
		}
	}

	void dragLeft()
	{
		if (disp_)
		{
			btn_l_push_ = true;
			btn_l_time_ = 0;
			move_ = true;
			move_time_ = 0;
		}
	}

	void dragRight()
	{
		if (disp_) {
			btn_r_push_ = true;
			btn_r_time_ = 0;
			move_ = true;
			move_time_ = 0;
		}
	}
	
};

}

#endif
