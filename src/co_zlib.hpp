
#pragma once

//
// Deflateによるデータ圧縮
// SOURCE:http://zlib.net/
//

#include <vector>
#include <fstream>
#include <zlib.h>


namespace ngs {

enum {
	OUTBUFSIZ = 1024 * 16,
};

void zlibEncode(std::vector<char>& output, const std::vector<char>& input)
{
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree	 = Z_NULL;
	z.opaque = Z_NULL;
	deflateInit(&z, Z_DEFAULT_COMPRESSION);

	char outbuf[OUTBUFSIZ];
	const void *in = &input[0];
	z.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(in));
	z.avail_in = input.size();
	void *out = outbuf;
	z.next_out = static_cast<Bytef *>(out);
	z.avail_out = OUTBUFSIZ;
	while (1)
	{
		int status = deflate(&z, Z_FINISH);
		if (status == Z_STREAM_END) break;
		if (z.avail_out == 0)
		{
			output.insert(output.end(), &outbuf[0], &outbuf[OUTBUFSIZ]);
			void *out = outbuf;
			z.next_out = static_cast<Bytef *>(out);
			z.avail_out = OUTBUFSIZ;
		}
	}

	int count = OUTBUFSIZ - z.avail_out;
	if (count != 0)
	{
			output.insert(output.end(), &outbuf[0], &outbuf[count]);
	}
	deflateEnd(&z);
}

void zlibDecode(std::vector<char>& output, const std::vector<char>& input)
{
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree	 = Z_NULL;
	z.opaque = Z_NULL;
	inflateInit(&z);
	
	char outbuf[OUTBUFSIZ];
	const void *in = &input[0];
	z.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(in));
	z.avail_in = input.size();
	void *out = outbuf;
	z.next_out = static_cast<Bytef *>(out);
	z.avail_out = OUTBUFSIZ;
	while (1)
	{
		int status = inflate(&z, Z_NO_FLUSH);
		if (status == Z_STREAM_END) break;
		if (z.avail_out == 0)
		{
			output.insert(output.end(), &outbuf[0], &outbuf[OUTBUFSIZ]);
			void *out = outbuf;
			z.next_out = static_cast<Bytef *>(out);
			z.avail_out = OUTBUFSIZ;
		}
	}

	int count = OUTBUFSIZ - z.avail_out;
	if (count != 0)
	{
			output.insert(output.end(), &outbuf[0], &outbuf[count]);
	}
	inflateEnd(&z);
}


void zlibWrite(const std::string& file, const std::vector<char>& input)
{
	std::vector<char> output;
	zlibEncode(output, input);
	std::ofstream fstr(file.c_str(), std::ios::binary);
	if (fstr.is_open())
	{
		std::copy(output.begin(), output.end(), std::ostreambuf_iterator<char>(fstr));
		// SOURCE:http://blogs.wankuma.com/episteme/archive/2009/01/09/166002.aspx
	}
}

void zlibRead(const std::string& file, std::vector<char>& output)
{
	std::ifstream fstr(file.c_str(), std::ios::binary);
	if (fstr.is_open())
	{
//		std::vector<char> input((std::istreambuf_iterator<char>(fstr)), (std::istreambuf_iterator<char>()));
		std::size_t size = fstr.seekg(0, std::ios::end).tellg();
		std::vector<char> input(size);
		fstr.seekg(0, std::ios::beg).read(&input[0], size);
		zlibDecode(output, input);
	}
}

}
