
#pragma once

//
// JSON読むだけ
//

#include <string>
#include <sstream>
#include <fstream>
#include "picojson.h"
#include "co_zlib.hpp"


namespace ngs {


bool checkZiped(std::string& res, const std::string& path)
{
	std::string::size_type index = path.rfind(".json");
	if(index != std::string::npos)
	{
		res = path.substr(0, index) + ".dz";
		// const std::string ext = ".dz";
		// res.replace(index, ext.length(), ext);
		return  isFileExists(res);
	}
	return false;
}


class Json {
	std::string filename_;
	picojson::value json_;

	void read_stream(std::basic_istream<char> *stream)
	{
		*stream >> json_;
#ifdef _DEBUG
		std::string err = picojson::get_last_error();
		if (!err.empty())
		{
			DOUT << "json:" << err << std::endl;
		}
#endif
	}

public:
	explicit Json(const std::string& filename) :
		filename_(filename)
	{
		DOUT << "Json()" << std::endl;
		reload();
	}
	
	~Json() {
		DOUT << "~Json()" << std::endl;
	}

	void reload()
	{
#ifdef READ_FROM_Z
		std::string f;
		if (checkZiped(f, filename_))
		{
			std::vector<char> output;
			zlibRead(f, output);

			const std::string s(&output[0], output.size());
			std::istringstream in(s);
			read_stream(&in);
		}
		else
#endif
		{
			std::ifstream fstr(filename_.c_str());
			if(fstr.is_open())
			{
				read_stream(&fstr);
			}
			else
			{
				DOUT << "Json::read error:" << filename_ << std::endl;
			}
		}
	}

	void reload(const std::string& file)
	{
		json_.get<picojson::object>().clear();
		filename_ = file;
		reload();
	}

	void write(const std::string file)
	{
#ifdef WRITE_TO_Z
		std::ostringstream sstr;
		sstr << json_;
		const std::string& str = sstr.str();
		std::vector<char> output(str.begin(), str.end());

		std::string::size_type index = file.rfind(".json");
		std::string dzfile;
		if(index != std::string::npos)
		{
			dzfile = file.substr(0, index) + ".dz";
		}
		else
		{
			DOUT << "json file error:" << file << std::endl;
			dzfile = file + ".dz";
		}
		zlibWrite(dzfile, output);
#else
		std::ofstream fstr(file.c_str());
		if (fstr.is_open()) { fstr << json_; };
#endif		
	}
	
	picojson::value& value() { return json_; }
};

}
