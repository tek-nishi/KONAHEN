
#pragma once

//
// ローカライズ処理
//

#include <iostream>
#include <string>
#include <map>

namespace ngs {

class Localize
{
	std::map<std::string, std::string> texts_;
#ifdef _DEBUG
	std::string file_;
	std::string path_;
#endif

	void read(const std::string& file, const std::string& path)
	{
		const std::string f = path + "devdata/" + file;
		std::ifstream fstr(f.c_str());
		if (fstr.is_open())
		{
			while(!fstr.eof())
			{
				std::string str;
				std::getline(fstr, str);
				if (str.size() > 0)
				{
					size_t pos = str.find_first_of("\t");
					std::string key = str.substr(0, pos);
					std::string value = str.substr(pos + 1);
#ifdef _DEBUG
					if (pos == str.npos)
					{
						DOUT << "Localize:text error:" << str << std::endl;
					}
					if (texts_.find(key) != texts_.end())
					{
						DOUT << "Localize:multi keyword:" << key << std::endl;
					}
#endif
					texts_.insert(std::map<std::string, std::string>::value_type(key, value));
				}
			}
		}
	}
	
public:
	Localize(const std::string& file, const std::string& path)
	{
		DOUT << "Localize()" << std::endl;

#ifdef _DEBUG
		file_ = file;
		path_ = path;
#endif
		read(file, path);
	}

	~Localize()
	{
		DOUT << "~Localize()" << std::endl;
	}

	const std::string& get(const std::string& text) const
	{
		std::map<std::string, std::string>::const_iterator it = texts_.find(text);
		if(it == texts_.end())
		{
			DOUT << "Localize:can't find:" << text << std::endl;
			return text;
		}
		return it->second;
	}

#ifdef _DEBUG
	void reload()
	{
		texts_.clear();
		read(file_, path_);
	}

	void reload(const std::string& file)
	{
		file_ = file;
		reload();
	}
#endif
};

}
