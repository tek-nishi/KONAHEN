
#pragma once

//
// フォント管理
//

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "co_font.hpp"


namespace ngs {

class FntMng {
public:
	typedef std::tr1::shared_ptr<Font> FontPtr;

private:
	std::map<std::string, FontPtr> objs_;

public:
	FntMng() {
		DOUT << "FntMng()" << std::endl;
	}
	~FntMng() {
		DOUT << "~FntMng()" << std::endl;
	}

	FontPtr read(const std::string& name, const int size, const std::string& path, const int type = FONT_TEXTURE)
	{
		std::map<std::string, FontPtr>::iterator it = objs_.find(name);
		if (it == objs_.end())
		{
			FontPtr obj(new Font(type, path, size));
			objs_[name] = obj;
			DOUT << "Read font:" << path << std::endl;
			return obj;
		}
		else
		{
			return it->second;
		}
	}
	
	FontPtr get(const std::string& name)
	{
		std::map<std::string, FontPtr>::iterator it = objs_.find(name);
		if (it == objs_.end())
		{
			DOUT << "No font obj:" << name << std::endl;
			return FontPtr();
		}
		else
		{
			return it->second;
		}
	}
};

}
