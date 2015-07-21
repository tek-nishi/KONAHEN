
#pragma once

//
// ゲーム内での操作を記録
//

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "co_task.hpp"


namespace ngs {

class Replay : public TaskProc
{
	GameEnv& env_;
	bool active_;
	
	bool record_;
	bool replay_;
	bool first_frame_;

	float time_;

	struct Info {
		float time;
		Quat<float> rot;
		float dist;
		bool touch;
		Vec3<float> pos;
	};
	Info info_;
	std::vector<Info> infoes_;
	float rot_;
	unsigned int rseed_;

	std::size_t index_;

	void read_stream(std::basic_istream<char> *stream)
	{
		DOUT << "Replay::read_stream()" << std::endl;

		*stream >> rot_ >> rseed_;
		while (!stream->eof())
		{
			Info info;
			*stream >> info.time
						 >> info.rot.x >> info.rot.y >> info.rot.z >> info.rot.w
						 >> info.dist >> info.touch
						 >> info.pos.x >> info.pos.y >> info.pos.z;
			infoes_.push_back(info);
		}
	}
	
public:
	explicit Replay(GameEnv& env) :
		env_(env),
		active_(true),
		record_(),
		replay_(),
		first_frame_(),
		time_(),
		info_(),
		rot_(),
		rseed_(),
		index_()
	{
		DOUT << "Replay()" << std::endl;
		infoes_.reserve(256);
	}

	~Replay()
	{
		DOUT << "~Replay()" << std::endl;
	}

	bool active() const { return active_; }

	const bool record() const { return record_; }
	void record(bool record)
	{
		record_ = record;
		if (record)
		{
			time_ = 0;
			rot_ = env_.earth->rotate();
			rseed_ = env_.rseed;
			infoes_.clear();
		}
	}

	const bool replay() const { return replay_; }
	void replay(bool replay)
	{
		replay_ = replay;
		if (replay)
		{
			time_ = 0;
			index_ = 0;
			first_frame_ = true;
		}
	}

	unsigned int& rseed() { return rseed_; }


	void write_stream(std::basic_ostream<char> *stream)
	{
		*stream << rot_ << " " << rseed_ << std::endl;
		for (std::vector<Info>::const_iterator it = infoes_.begin(); it != infoes_.end(); ++it)
		{
			*stream << it->time << " "
							<< it->rot.x << " " << it->rot.y << " " << it->rot.z << " " << it->rot.w << " "
							<< it->dist << " " << it->touch << " "
							<< it->pos.x << " " << it->pos.y << " " << it->pos.z << std::endl;
		}
	}
	
	void write()
	{
		std::string fpath = *env_.savePath + "replay.rep";
		std::ofstream fstr(fpath.c_str());
		if (fstr.is_open()) write_stream(&fstr);
	}
	
	void read(const std::string& file)
	{
		infoes_.clear();

#ifdef READ_FROM_Z
		if (isFileExists(*env_.path + "devdata/" + file + ".dz"))
		{
			std::vector<char> output;
			zlibRead(*env_.path + "devdata/" + file + ".dz", output);

			const std::string s(&output[0], output.size());
			std::istringstream in(s);
			read_stream(&in);
		}
		else
#endif
		{
			std::string fpath = *env_.path + "devdata/" + file + ".rep";
			std::ifstream fstr(fpath.c_str());
			if (fstr.is_open()) read_stream(&fstr);
		}
	}
	
	void step(const float delta_time)
	{
		time_ += delta_time;
		if (record_)
		{
			info_.time = time_;
			info_.rot = env_.camera->getRot();
			info_.dist = env_.camera->getDist();
			infoes_.push_back(info_);
			info_.touch = false;
		}
		else
		if (replay_)
		{
			while((index_ < infoes_.size()) && (infoes_[index_].time <= time_))
			{
				if (first_frame_)
				{
					first_frame_ = false;
					env_.earth->rotate(rot_);
				}
				env_.camera->setRot(infoes_[index_].rot);
				env_.camera->setDist(infoes_[index_].dist);
				
				if (infoes_[index_].touch)
				{
					env_.hit_pos = infoes_[index_].pos;
					env_.task->sendMsgAll(MSG_GAME_TOUCH);
				}
				++index_;
			}
		}
	}

	void draw() {}

	void msg(const int msg)
	{
		switch (msg)
		{
		case MSG_GAME_TOUCH:
			if (record_)
			{
				info_.touch = true;
				info_.pos = env_.hit_pos;
			}
			break;
		}
	}
	
};

}
