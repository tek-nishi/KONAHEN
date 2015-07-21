
#pragma once

//
// 2D描画セットアップ
//

#include <iostream>
#include <string>
#include "co_task.hpp"
#include "nn_gameenv.hpp"


namespace ngs {

class Game2DSetup : public TaskProc {
	GameEnv& env_;
	bool active_;

public:
	explicit Game2DSetup(GameEnv& env) :
		env_(env),
		active_(true)
	{
		DOUT << "Game2DSetup()" << std::endl;
	}
	~Game2DSetup() {
		DOUT << "~Game2DSetup()" << std::endl;
	}
	
	bool active() const { return active_; }
	
	void step(const float delta_time) {}

	void draw()
	{
		env_.cockpit->setup();
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
	}

	void msg(const int msg) {}

};

}

