
#pragma once

//
// テクスチャ管理
//

#include <iostream>
#include <string>
#include <map>
#include "co_texture.hpp"


namespace ngs {

class TexMng {
public:
	typedef std::tr1::shared_ptr<Texture> tex_ptr;

private:
	std::map<std::string, tex_ptr> tex_obj_;
		
public:
	TexMng() {
		DOUT << "TexMng()" << std::endl;
	}
	~TexMng() {
		DOUT << "~TexMng()" << std::endl;
	}

	tex_ptr read(const std::string& path)
	{
		std::string name = getFileName(path);
		std::map<std::string, tex_ptr>::iterator it = tex_obj_.find(name);
		if(it == tex_obj_.end())
		{
			// DOUT << "texmng read: " << path << std::endl;
			tex_ptr obj(new Texture(path));
			tex_obj_[name] = obj;
			return obj;
		}
		else
		{
			return it->second;
		}
	}

	tex_ptr get(const std::string& name)
	{
		std::map<std::string, tex_ptr>::iterator it = tex_obj_.find(name);
		if(it == tex_obj_.end())
		{
			DOUT << "No texture obj:" << name << std::endl;
			 return tex_ptr();
		}
		else
		{
			return it->second;
		}
	}
};

}
