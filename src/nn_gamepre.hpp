
#pragma once

//
// ゲーム起動が重いので直前に空の処理を挟む
//

#include <string>
#include "co_texmng.hpp"
#include "nn_camera.hpp"
#include "nn_proc_base.hpp"


namespace ngs {

class GamePre : public ProcBase
{
	int delay_;
	TexMng::tex_ptr texture_;
	Camera cockpit_;

public:
	GamePre(const Vec2<int>& size, const std::string& path) :
		delay_ (2),
		texture_(new Texture(path + "devdata/wait.png")),
		cockpit_(Camera::ORTHOGONAL)
	{
		DOUT << "GamePre()" << std::endl;
		cockpit_.setSize(size.x, size.y);
	}
	~GamePre() {
		DOUT << "~GamePre()" << std::endl;
	}

	void resize(const int w, const int h) {}
	void resize(const int w, const int h, const float sx, const float sy) {
		cockpit_.setSize(w, h);
	}
	void y_bottom(const float y) {}
	
	bool step(const float delta_time)
	{
		return --delay_ > 0;
	}
	
	void draw()
	{
		cockpit_.setup();
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		GrpSprite obj;
		obj.size(192,32);
		obj.center();
		obj.texture(texture_);
		obj.uv(0, 0);
		obj.col(1, 1, 1, 1);
		obj.draw();
	}

#ifdef _DEBUG
	void forceFrame(const bool force) {}
#endif
};
		
}
