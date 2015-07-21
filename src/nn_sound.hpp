
#pragma once

//
// サウンド管理
// TODO: F.I. F.O.
//

#include <iostream>
#include <string>
#include <vector>
#include "co_audio.hpp"


namespace ngs {

class Sound {
	struct Request {
		std::string name;
		float gain;
		float delay;
		bool played;
	};

	Audio& audio_;

	std::vector<Request> req_;

	struct cleanupProc {
		bool operator()(const Request& obj) const {
			return obj.played;
		}
	};

public:
	explicit Sound(Audio& audio) :
		audio_(audio)
	{
		DOUT << "Sound()" << std::endl;
	}
	
	~Sound()
	{
		DOUT << "~Sound()" << std::endl;
		stopAll();
	}
	
	void update(const float delta_time)
	{
		for (std::vector<Request>::iterator it = req_.begin(); it != req_.end(); ++it)
		{
			if ((it->delay -= delta_time) <= 0.0)
			{
				audio_.play(it->name, it->gain);
				it->played = true;
			}
		}

		{
			std::vector<Request>::iterator end = std::remove_if(req_.begin(), req_.end(), cleanupProc());
			req_.erase(end, req_.end());
			// TIPS:↑remove_ifはコンテナのサイズを変えないので、こう実装する
		}
	}

	void play(const std::string& name, const float gain = 1.0, const float delay = 0.0)
	{
		if(delay == 0.0)
		{
			audio_.play(name, gain);
		}
		else
		{
			Request req;
			req.name = name;
			req.gain = gain;
			req.delay = delay;
			req.played = false;
			req_.push_back(req);
		}
	}

	void stop(const std::string& name)
	{
		audio_.stop(name);
	}

	void stopAll()
	{
		audio_.stopAll();
		req_.clear();
	}

	void setGain(const float gain)
	{
		audio_.setGain(gain);
	}
	const float getGain() const { return audio_.getGain(); }

	void mute(const u_int type, const bool mute)
	{
		audio_.mute(type, mute);
	}
	bool mute(const u_int type) const { return audio_.mute(type); }

};

}
