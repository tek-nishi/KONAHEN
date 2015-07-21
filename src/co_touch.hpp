
#pragma once

// 
// iOSのタッチに特化した入力判定
// 

#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <fstream>
#include "co_vec2.hpp"


namespace ngs {

struct TouchInfo {
	Vec2<float> pos;
	// タッチ座標
	Vec2<float> l_pos;
	// 直前のタッチ座標
};
// FIXME:const 属性のメンバを含むクラスをコンテナに入れたい

class Touch;
// FIXME: 先行宣言しておく必要がある

class TouchCallBack {
public:
	virtual void touchStart(const Touch& touch, const std::vector<TouchInfo>& info) = 0;
	virtual void touchMove(const Touch& touch, const std::vector<TouchInfo>& info) = 0;
	virtual void touchEnd(const Touch& touch, const std::vector<TouchInfo>& info) = 0;
};


class Touch {
	Vec2<float> size_;																// 取り得る座標
	Vec2<float> disp_ofs_;														// 表示時のオフセット
	Vec2<float> disp_size_;														// 表示時のサイズ
	
	std::vector<TouchCallBack *> cb_;

#ifdef TOUCH_RECORD
	std::string file_;
	u_int frame_;
	enum {
		START, MOVE, END
	};
	struct TouchData {
		u_int frame;
		u_int type;
		std::vector<TouchInfo> info;
	};
	std::vector<TouchData> data_;
	bool record_;
	bool replay_;
	u_int offset_;
#endif
	
public:
	Touch() :
		size_(640.0, 480.0),
		disp_size_(640.0, 480.0)
	{
		DOUT << "Touch()" << std::endl;
#ifdef TOUCH_RECORD
		frame_ = 0;
		record_ = 0;
		replay_ = 0;
		offset_ = 0;
#endif
	}
	
	~Touch() {
		DOUT << "~Touch()" << std::endl;
	}
	
	void resize(const Vec2<float>& size, const Vec2<float>& disp_ofs, const Vec2<float>& disp_size)
	{
		size_ = size;
		disp_ofs_ = disp_ofs;
		disp_size_ = disp_size;
	}

	Vec2<float> pos2local(const Vec2<float>& pos) const
	{
		float x = (pos.x - disp_ofs_.x) * (size_.x / (disp_size_.x - (disp_ofs_.x * 2.0f))) - size_.x / 2.0f;
		float y = (pos.y - disp_ofs_.y) * (size_.y / (disp_size_.y - (disp_ofs_.y * 2.0f))) - size_.y / 2.0f; 
		return Vec2<float>(x, y);
	}

	Vec2<float> pos2window(const Vec2<float>& pos) const
	{
		return Vec2<float>(pos.x, disp_size_.y - pos.y);
	}
	
	void resistCallBack(TouchCallBack *const cb)
	{
		cb_.push_back(cb);
	}

	void removeCallBack(const TouchCallBack *const cb)
	{
		std::vector<TouchCallBack *>::iterator end = std::remove_if(cb_.begin(), cb_.end(), std::bind2nd(std::equal_to<const TouchCallBack *>(), cb));
		cb_.erase(end, cb_.end());
	}

	void touchStart(const std::vector<TouchInfo>& info)
	{
#ifdef TOUCH_RECORD
		if (record_)
		{
			TouchData data = { frame_, START, info };
			data_.push_back(data);
			frame_ = 0;
		}
#endif
		
		for (std::vector<TouchCallBack *>::iterator it = cb_.begin(); it != cb_.end(); ++it)
		{
			(*it)->touchStart(*this, info);
		}
	}

	void touchMove(const std::vector<TouchInfo>& info)
	{
#ifdef TOUCH_RECORD
		if (record_)
		{
			TouchData data = { frame_, MOVE, info };
			data_.push_back(data);
			frame_ = 0;
		}
#endif

		for (std::vector<TouchCallBack *>::iterator it = cb_.begin(); it != cb_.end(); ++it)
		{
			(*it)->touchMove(*this, info);
		}
	}

	void touchEnd(const std::vector<TouchInfo>& info)
	{
#ifdef TOUCH_RECORD
		if (record_)
		{
			TouchData data = { frame_, END, info };
			data_.push_back(data);
			frame_ = 0;
		}
#endif

		for(std::vector<TouchCallBack *>::iterator it = cb_.begin(); it != cb_.end(); ++it)
		{
			(*it)->touchEnd(*this, info);
		}
	}
	// FIXME: システム側でTouchInfoを作らなければならない


#ifdef TOUCH_RECORD

	void step()
	{
		++frame_;
		if (replay_)
		{
			if (frame_ >= data_[offset_].frame)
			{
				switch(data_[offset_].type)
				{
				case START:
					touchStart(data_[offset_].info);
					break;

				case MOVE:
					touchMove(data_[offset_].info);
					break;

				case END:
					touchEnd(data_[offset_].info);
					break;
				}
				frame_ = 0;
				++offset_;
				if (offset_ >= data_.size())
				{
					offset_ = 0;
				}
			}
		}
	}

	void write(const std::string& file)
	{
		std::ofstream fstr(file.c_str());
		for (std::vector<TouchData>::iterator it = data_.begin(); it != data_.end(); ++it)
		{
			fstr << it->frame << " "
					 << it->type << " "
					 << it->info.size();
			for (std::vector<TouchInfo>::iterator itt = it->info.begin(); itt != it->info.end(); ++itt)
			{
				fstr << " "
						 << itt->pos.x << " " << itt->pos.y
						 << " "
						 << itt->l_pos.x << " " << itt->l_pos.y;
			}
			fstr << std::endl;
		}
		DOUT << "Touch data write." << std::endl;
	}

	void read(const std::string& file)
	{
		std::ifstream fstr(file.c_str());
		if (fstr.is_open())
		{
			data_.clear();
			while (!fstr.eof())
			{
				TouchData data;
				fstr >> data.frame >> data.type;

				u_int num;
				fstr >> num;
				for (u_int i = 0; i < num; ++i)
				{
					TouchInfo info;
					fstr >> info.pos.x >> info.pos.y
							 >> info.l_pos.x >> info.l_pos.y;
					data.info.push_back(info);
				}
				data_.push_back(data);
			}
			DOUT << "Touch data read." << std::endl;
		}
	}

	void file(const std::string& file)
	{
		file_ = file;
	}

	// 記録
	void record(const bool flag)
	{
		replay_ = false;
		if (flag)
		{
			data_.clear();
			frame_ = 0;
		}
		else
		if (record_)
		{
			write(file_);
		}
		record_ = flag;
	}
	bool record() const { return record_; }

	// 再生
	void replay(const bool flag)
	{
		replay_ = flag;
		record_ = false;
		if (flag)
		{
			read(file_);
			frame_ = 0;
			offset_ = 0;
		}
	}
	bool replay() const { return replay_; }

#endif
	
};

}
