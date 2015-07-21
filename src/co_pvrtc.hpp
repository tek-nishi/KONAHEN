
#pragma once

//
// PVRTCテクスチャを読み込む
// iOS専用
// TODO: reinterpret_cast 排除
// 

#if (TARGET_OS_IPHONE)

#include <libkern/OSByteOrder.h>


namespace ngs {

class Pvrtc
{
	enum Constants
	{
		PVR_FLAG_TYPE_PVRTC_2 = 24,
		PVR_FLAG_TYPE_PVRTC_4 = 25,
		PVR_FLAG_TYPE_MASK    = 0xff,
		PVR_MAX_SURFACES      = 16
	};

	struct Header
	{
		uint32_t headerSize;
		uint32_t height;
		uint32_t width;
		uint32_t numMipmaps;
		uint32_t flags;
		uint32_t dataSize;
		uint32_t bpp;
		uint32_t bitmaskRed;
		uint32_t bitmaskGreen;
		uint32_t bitmaskBlue;
		uint32_t bitmaskAlpha;
		uint32_t tag;
		uint32_t numSurfaces;
	};
	
	struct Surface
	{
		uint32_t		height;
		uint32_t		width;
		GLuint      size;
		const void* bits;
	};

	GLuint  width_;
	GLuint  height_;
	GLenum  format_;
	bool    hasAlpha_;
	bool		mipmap_;
	Surface surfaces_[PVR_MAX_SURFACES];
	GLuint  numSurfaces_;

	Pvrtc() {}
public:
	explicit Pvrtc(const char *buffer) :
		numSurfaces_()
	{
		DOUT << "Pvrtc()" << std::endl;
		const Header& header = *reinterpret_cast<const Header *>(buffer);
		uint32_t tag = OSSwapLittleToHostInt32(header.tag);

		const char identifier[4] = { 'P', 'V', 'R', '!' };

    if (identifier[0] != ((tag >>  0) & 0xff) ||
       identifier[1] != ((tag >>  8) & 0xff) ||
       identifier[2] != ((tag >> 16) & 0xff) ||
       identifier[3] != ((tag >> 24) & 0xff))
    {
			DOUT << "error identifier:" << tag << std::endl;
			return;
    }

    uint32_t flags       = OSSwapLittleToHostInt32(header.flags);
    uint32_t formatFlags = flags & PVR_FLAG_TYPE_MASK;
    if (formatFlags == PVR_FLAG_TYPE_PVRTC_4 || formatFlags == PVR_FLAG_TYPE_PVRTC_2)
    {
			width_    = OSSwapLittleToHostInt32(header.width);
			height_   = OSSwapLittleToHostInt32(header.height);
			hasAlpha_ = OSSwapLittleToHostInt32(header.bitmaskAlpha) ? true : false;
			mipmap_   = (OSSwapLittleToHostInt32(header.numMipmaps) > 0) ? true : false;
			int bitShift = (formatFlags == PVR_FLAG_TYPE_PVRTC_4) ? 1 : 2;			
			if (hasAlpha_)
			{
				format_		= (formatFlags == PVR_FLAG_TYPE_PVRTC_4) ? GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			}
			else
			{
				format_		= (formatFlags == PVR_FLAG_TYPE_PVRTC_4) ? GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			}

			GLuint         w      = width_;
			GLuint         h      = height_;
			GLuint         offset = 0;
			GLuint         size   = OSSwapLittleToHostInt32(header.dataSize);
			const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(buffer) + sizeof(header);
			while (offset < size && numSurfaces_ < PVR_MAX_SURFACES)
			{
				Surface& surface = surfaces_[numSurfaces_];
				numSurfaces_ += 1;

				int surfaceSize = (w * h) >> bitShift;
				if (surfaceSize < 32) surfaceSize = 32;

				surface.width	 = w;
				surface.height = h;
				surface.size	 = surfaceSize;
				surface.bits	 = &pBytes[offset];

				(w >>= 1) || (w = 1);
				(h >>= 1) || (h = 1);

				offset += surfaceSize;
			}
			DOUT << "PVRTC:" << width_ << "x" << height_ << " Alpha:" << hasAlpha_ << " mipmap:" << mipmap_ << " num:" << numSurfaces_ << std::endl;
    }
    else
    {
			DOUT << "error format" << std::endl;
			return;
    }
	
	}

	Vec2<int> size() const { return Vec2<int>(width_, height_); }
	int width() const { return width_; }
	int height() const { return height_; }
	bool mipmap() const { return mipmap_; }
	
	bool submit()
	{
    if (numSurfaces_ <= 0) return false;
		
    for (GLuint i = 0 ; i < numSurfaces_ ; ++i)
    {
			const Surface& surface = surfaces_[i];
			glCompressedTexImage2D(GL_TEXTURE_2D, i, format_, surface.width, surface.height, 0, surface.size, surface.bits);
    }
		
    return true;
	}
};

}

#endif
