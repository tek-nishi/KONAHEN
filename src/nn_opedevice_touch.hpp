
#if (TARGET_OS_IPHONE)

#pragma once

//
// DEMO画面での操作表示
// マウス版
//
namespace ngs {

class OpeDevice {
	picojson::object& params_;
	const float ofs_x_, ofs_y_;

	WidgetsMap::WidgetPtr l_body_;
	WidgetsMap::WidgetPtr r_body_;
	WidgetsMap::WidgetPtr touch_;
	WidgetsMap::WidgetPtr l_drag_;
	WidgetsMap::WidgetPtr r_drag1_;
	WidgetsMap::WidgetPtr r_drag2_;

	WidgetsMap::WidgetPtr body_;

	bool disp_;
	float frame_;

	float ofs_eft_x_;
	float ofs_eft_time_, ofs_eft_time_ed_;

	bool manu_;
	float manu_time_;
	float manu_time_ed_;
	
	bool touch_disp_;
	float touch_alpha_;
	float touch_alpha_st_, touch_alpha_ed_;
	float touch_time_, touch_time_ed_;

	bool drag_l_disp_;
	float drag_l_time_;
	float drag_l_time_ed_;

	bool drag_r_disp_;
	float drag_r_time_;
	float drag_r_time_ed_;

public:
	explicit OpeDevice(GameEnv& env) :
		params_(env.params->value().get<picojson::object>()["title"].get<picojson::object>()),
		ofs_x_(params_["operate_x"].get<double>()),
		ofs_y_(params_["operate_y"].get<double>()),
		disp_(),
		frame_(),
		ofs_eft_time_(),
		ofs_eft_time_ed_(0.8),
		manu_(),
		manu_time_(),
		manu_time_ed_(1),
		touch_disp_(),
		touch_alpha_(),
		touch_alpha_st_(1.0),
		touch_alpha_ed_(0.0),
		touch_time_(),
		touch_time_ed_(1.0),
		drag_l_disp_(),
		drag_l_time_ed_(1.0),
		drag_r_disp_(),
		drag_r_time_ed_(1.0)
		
	{
		DOUT << "OpeDevice()" << std::endl; 

		const WidgetsMap widgets(params_["opedevice_touch_widgets"].get<picojson::object>(), env.texMng, *env.path, env.size, env.y_bottom);
		l_body_ = widgets.get("l_body");
		r_body_ = widgets.get("r_body");
		touch_ = widgets.get("touch");
		l_drag_ = widgets.get("l_drag");
		r_drag1_ = widgets.get("r_drag1");
		r_drag2_ = widgets.get("r_drag2");

		body_ = r_body_;
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

		if (manu_)
		{
			manu_time_ += delta_time;
			manu_ = manu_time_ < manu_time_ed_;
			if (!manu_) body_ = r_body_;
		}

		if (touch_disp_)
		{
			touch_time_ += delta_time;
			easing.ease(touch_alpha_, touch_time_, touch_alpha_st_, touch_alpha_ed_, touch_time_ed_);
			touch_disp_ = touch_time_ < touch_time_ed_;
		}
		if (drag_l_disp_)
		{
			drag_l_time_ += delta_time;
			drag_l_disp_ = drag_l_time_ < drag_l_time_ed_;
		}
		if (drag_r_disp_)
		{
			drag_r_time_ += delta_time;
			drag_r_disp_ = drag_r_time_ < drag_r_time_ed_;
		}
		frame_ += delta_time;
	}

	void draw()
	{
		glPushMatrix();
		glTranslatef(ofs_x_ + ofs_eft_x_, ofs_y_, 0);
		
		if (touch_disp_)
		{
			touch_->setCol(1, 0, 0, touch_alpha_);
			touch_->draw();
		}

		if (drag_l_disp_ && ((int)(frame_ * 2.0) & 1))
		{
			l_drag_->setCol(1, 0, 0, 1);
			l_drag_->draw();
		}

		if (drag_r_disp_)
		{
			WidgetsMap::WidgetPtr r_drag = ((int)(frame_ * 2.0) & 1) ? r_drag1_ : r_drag2_;
			r_drag->setCol(1, 0, 0, 1);
			r_drag->draw();
		}

		body_->draw();

		glPopMatrix();
	}
	
	void touchLeft()
	{
		if (disp_)
		{
			touch_disp_ = true;
			drag_l_disp_ = false;
			drag_r_disp_ = false;

			touch_time_ = 0;
			body_ = l_body_;
			manu_ = true;
			manu_time_ = 0;
		}
	}

	void dragLeft()
	{
		if (disp_)
		{
			touch_disp_ = false;
			drag_l_disp_ = true;
			drag_r_disp_ = false;

			drag_l_time_ = 0;
			body_ = l_body_;
			manu_ = true;
			manu_time_ = 0;
		}
	}

	void dragRight()
	{
		if (disp_)
		{
			touch_disp_ = false;
			drag_l_disp_ = false;
			drag_r_disp_ = true;

			drag_r_time_ = 0;
			body_ = l_body_;
			manu_ = true;
			manu_time_ = 0;
		}
	}
	
};

}

#endif
