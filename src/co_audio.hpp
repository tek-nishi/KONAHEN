
#pragma once

//
// OpenALによるサウンド管理
// ※量子化ビットは16固定
//

#if defined (__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#if defined (_MSC_VER)
#pragma comment (lib, "openal32.lib")
#endif

#include <string>
#include <map>
#include <algorithm>
#include "co_pcm.hpp"
#include "co_json.hpp"


namespace ngs {

const char *AudioErrorString(const GLenum err)
{
	static const struct Err {
		const GLenum err;
		const char *str;
	} tbl[] = {
		{ AL_INVALID_NAME, "不正なネームの引数です" },
		{ AL_INVALID_ENUM, "不正な列挙値の引数です" },
		{ AL_INVALID_VALUE, "不正な引数です" },
		{ AL_INVALID_OPERATION, "禁止されている呼び出しです" },
		{ AL_OUT_OF_MEMORY, "メモリを割り当てる事が出来ません" },

		{ AL_NO_ERROR, "エラー番号が不正です" }
	};
	int i;
	for (i = 0; i < elemsof(tbl); ++i)
	{
		if (tbl[i].err == err) break;
	}
	assert(i != elemsof(tbl));
	if (i == elemsof(tbl)) --i;												// エラー番号が不正だった時の対策
	
	return tbl[i].str;
}


class Audio {
public:
	enum {
		BGM,
		SE
	};
	
private:
	struct SndObj {
		int id;
		int ch;
		bool loop;
		float gain;
		u_int type;
	};

	ALCdevice *const device_;
	ALCcontext *const context_;

	float gain_;
	std::vector<u_int> mute_;

	std::vector<ALuint> buffer_;
	std::vector<ALuint> source_;
	std::vector<float> ch_gain_;

	std::map<std::string, int> ch_;
	std::map<std::string, SndObj> obj_;
	std::map<int, std::vector<u_int> > type_;

	void setup(const std::string& path)
	{
		Json json(path + "devdata/params.json");
		picojson::object& params = json.value().get<picojson::object>()["audio"].get<picojson::object>();

		int num = params.size();
		buffer_.resize(num);
		alGenBuffers(num, &buffer_[0]);

		int idx = 0;
		int ch_max = 0;
		for(picojson::object::iterator it = params.begin(); it != params.end(); ++it)
		{
			picojson::object& obj = it->second.get<picojson::object>();
			std::string file = path + "devdata/" + obj["file"].get<std::string>();

			Pcm pcm(file);
			alBufferData(buffer_[idx], pcm.channel() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, pcm.data(), pcm.size(), pcm.rate());

			std::string ch = obj["ch"].get<std::string>();
			std::map<std::string, int>::iterator it_ch = ch_.find(ch);
			int ch_idx;
			if(it_ch == ch_.end())
			{
				ch_[ch] = ch_max;
				ch_idx = ch_max;
				++ch_max;
			}
			else
			{
				ch_idx = it_ch->second;
			}

			u_int type = obj["type"].get<double>();
			{
				std::map<int, std::vector<u_int> >::iterator it = type_.find(type);
				if (it == type_.end())
				{
					std::vector <u_int> v;
					v.push_back(ch_idx);
					type_.insert(std::map<int, std::vector<u_int> >::value_type(type, v));
				}
				else
				{
					std::vector<u_int>::iterator itt = std::find(it->second.begin(), it->second.end(), ch_idx);
					if (itt == it->second.end())
					{
						it->second.push_back(ch_idx);
					}
				}
				// チャンネルをタイプ別に積んでおく
			}

			SndObj sndobj;
			sndobj.id = idx;
			sndobj.ch = ch_idx;
			sndobj.loop = obj["loop"].get<bool>();
			sndobj.gain = obj["gain"].get<double>();
			sndobj.type = type;
			obj_[it->first] = sndobj;
			
			++idx;
		}

		source_.resize(ch_max);
		alGenSources(ch_max, &source_[0]);
		ch_gain_.resize(ch_max);
		std::fill(ch_gain_.begin(), ch_gain_.end(), 0.0);
	}

	void cleanup()
	{
		alDeleteSources(source_.size(), &source_[0]);
		alDeleteBuffers(buffer_.size(), &buffer_[0]);

		source_.clear();
		buffer_.clear();
	}
	
public:
	explicit Audio(const std::string& path) :
		device_(alcOpenDevice(0)),
		context_(alcCreateContext(device_, 0)),
		gain_(1.0)
	{
		DOUT << "Audio()" << std::endl;

		alcMakeContextCurrent(context_);								/* ここのエラーはスルーしても構わない */
		setup(path);

		mute_.push_back(0);
		mute_.push_back(0);
		// タイプ別ミュートの初期化
	}

	~Audio()
	{
		DOUT << "~Audio()" << std::endl;

		stopAll();
		cleanup();

		alcMakeContextCurrent(0);
		/* TIPS:解放する為にカレントコンテキストをNULLにする */
		alcDestroyContext(context_);
		alcCloseDevice(device_);
	}

	void play(const std::string& name, const float gain = 1.0)
	{
		std::map<std::string, SndObj>::iterator it = obj_.find(name);
		if(it == obj_.end())
		{
			DOUT << "No sound id:" << name << std::endl;
			return;
		}

		const SndObj& sndobj = it->second;

		if (mute_[sndobj.type])
		{
			return;
		}
		
		int ch = sndobj.ch;
		alSourceStop(source_[ch]);
		alSourcei(source_[ch], AL_BUFFER, buffer_[sndobj.id]);
		alSourcef(source_[ch], AL_GAIN, gain * sndobj.gain * gain_);
		alSourcei(source_[ch], AL_LOOPING, sndobj.loop);
		alSourcePlay(source_[ch]);
		ch_gain_[ch] = gain * sndobj.gain;
		DOUT << "audio gain:" << name << " " << gain * gain_ << std::endl;
	}

	void stop(const std::string& name)
	{
		std::map<std::string, int>::iterator it = ch_.find(name);
		if(it == ch_.end())
		{
			DOUT << "No ch id:" << name << std::endl;
			return;
		}

		int ch = it->second;
		alSourceStop(source_[ch]);
		alSourcei(source_[ch], AL_BUFFER, 0);
	}

	void stopAll()
	{
		int ch = 0;
		for(std::vector<ALuint>::iterator it = source_.begin(); it != source_.end(); ++it, ++ch)
		{
			alSourceStop(*it);
			alSourcei(*it, AL_BUFFER, 0);
			ch_gain_[ch] = 0.0;
		}
	}

	float getGain() const { return gain_; }
	void setGain(const float gain)
	{
		gain_ = gain;

		int ch = 0;
		for(std::vector<ALuint>::iterator it = source_.begin(); it != source_.end(); ++it, ++ch)
		{
			alSourcef(*it, AL_GAIN, ch_gain_[ch] * gain_);
		}
	}

	void mute(const u_int type, bool mute)
	{
		mute_[type] = mute ? 1 : 0;
		if (mute_[type])
		{
			std::map<int, std::vector<u_int> >::iterator it = type_.find(type);
			if (it == type_.end())
			{
				DOUT << "No mute type:" << type << std::endl;
			}
			else
			{
				for (std::vector<u_int>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt)
				{
					alSourceStop(source_[*itt]);
					alSourcei(source_[*itt], AL_BUFFER, 0);
					ch_gain_[*itt] = 0.0;
				}
				// ミュート対象のチャンネルをすべて止める
			}
		}
	}
	bool mute(const u_int type) const { return mute_[type] ? true : false; }

	ALCcontext *const context() const { return context_; }

};

}
