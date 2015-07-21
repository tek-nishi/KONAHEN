
#pragma once

//
// 色々テスト環境
//

#if !(TARGET_OS_IPHONE) && _DEBUG

#include <string>
#include "co_keyinp.hpp"
#include "co_easing.hpp"
#include "co_font.hpp"
#include "nn_camera.hpp"
#include "nn_proc_base.hpp"


namespace ngs {


class SandBox : public ProcBase, TouchCallBack
{
	Touch& touch_;
	Keyinp& keyinp_;
	Vec2<int> size_;
	Camera cockpit_;

	Font font_;

	Vec2<float> pos_;
	float radius_;

	struct point { Vec2<float> st; Vec2<float> ed; };
	std::vector<point> points_;

public:
	SandBox(const Vec2<int>& size, const float scale, Touch& touch, Keyinp& keyinp, const std::string& path) :
		touch_(touch),
		keyinp_(keyinp),
		size_(size),
		cockpit_(Camera::ORTHOGONAL),
		font_(FONT_TEXTURE, path + "devdata/VeraMono.ttf", 12),
		radius_(10)
	{
		DOUT << "SandBox()" << std::endl;
		cockpit_.setSize(size.x, size.y);

		for (int i = 0; i < 100; ++i)
		{
			point pos = {
				Vec2<float>((rand() % 800) - 400, (rand() % 800) - 400),
				Vec2<float>((rand() % 800) - 400, (rand() % 800) - 400)
			};
			points_.push_back(pos);
		}
		
		touch_.resistCallBack(this);
	}
	~SandBox() {
		DOUT << "~SandBox()" << std::endl;
		touch_.removeCallBack(this);
	}

	void resize(const int w, const int h) {}
	void resize(const int w, const int h, const float sx, const float sy) {
		cockpit_.setSize(w, h);
	}
	void y_bottom(const float y) {}
	
	bool step(const float delta_time)
	{
		return true;
	}

	void draw()
	{
		cockpit_.setup();
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);

		size_t size = points_.size();
		size_t i = 0;
		for(i = 0; i < size; ++i)
		{
			if (crossCircleLine(points_[i].st, points_[i].ed, pos_, radius_)) break;
		}
		bool cross = i < size;

		for(i = 0; i < size; ++i)
		{
			GrpLine obj;
			obj.points(points_[i].st, points_[i].ed);
			obj.col(0, 0, 1, 1);
			obj.draw();
		}

		{
			GrpCircle obj;
			obj.radius(radius_, radius_);
			obj.div(20);
			obj.col(cross ? GrpCol<float>(1, 0, 0, 1) : GrpCol<float>(0, 1, 0, 1));
			obj.pos(pos_);
			obj.draw();
		}
	}

#ifdef _DEBUG
	void forceFrame(const bool force) {}
#endif

	void touchStart(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		pos_ = touch_.pos2local(info[0].pos);
	}
	
	void touchMove(const Touch& touch, const std::vector<TouchInfo>& info)
	{
		pos_ = touch_.pos2local(info[0].pos);
	}
	
	void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info) {}
};
		
}

#endif
