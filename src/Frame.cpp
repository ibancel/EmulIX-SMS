#include "Frame.h"

#include <algorithm>

Frame::Frame(int width, int height)
    : _height{height},
      _width{width},
      _backdropColor(0,0,0,255),
      _pixels(width * height * 4, 255),
      _pixelsBottom(width * height * 4, 255)
{

}

Frame::Frame(int width, int height, const PixelColor& color)
    : Frame(width, height)
{
    for(int i = 0 ; i < _width*_height ; i++) {
        setPixel(i, color);
    }
}

Frame::~Frame()
{

}

const uint8_t* Frame::data(FrameOriginFormat iOriginFormat)
{
    if(iOriginFormat == FrameOriginFormat::kTopLeft) {
        return _pixels.data();
    }

    return _pixelsBottom.data();
}

PixelColor Frame::getBackdropColor()
{
    return _backdropColor;
}

PixelColor Frame::getPixelColor(int x, int y)
{
    uint8_t* ptr = &_pixels[(y*_width+x)*4];
    return *(reinterpret_cast<PixelColor*>(ptr));
}

void Frame::setBackdropColor(const PixelColor &iColor)
{
    _backdropColor = iColor;
}

void Frame::setPixel(int x, int y, const PixelColor& iColor)
{
    setPixel(posToPosIndex(x,y), iColor);
}

void Frame::setPixel(int iPixelIndex, const PixelColor& iColor)
{
    SetPixel(_pixels, pixelIndexToVectorIndex(iPixelIndex, FrameOriginFormat::kTopLeft), iColor);
    SetPixel(_pixelsBottom, pixelIndexToVectorIndex(iPixelIndex, FrameOriginFormat::kBottomLeft), iColor);
}

// Private

void Frame::SetPixel(std::vector<uint8_t>& ioPixels, int iPixelIndex, const PixelColor& iColor)
{
    ioPixels[iPixelIndex*4+0] = iColor.r;
    ioPixels[iPixelIndex*4+1] = iColor.g;
    ioPixels[iPixelIndex*4+2] = iColor.b;
    ioPixels[iPixelIndex*4+3] = iColor.a;
}

int Frame::pixelIndexToVectorIndex(int iPixelIndex, FrameOriginFormat iOriginFormat)
{
    if(iOriginFormat == FrameOriginFormat::kTopLeft) {
        return iPixelIndex;
    }

    int y = static_cast<int>(iPixelIndex / (_width > 0 ? _width : 1));
    int x = iPixelIndex - y*_width;
    return (_height-y-1)*_width + x;
}

int Frame::posToPosIndex(int x, int y, FrameOriginFormat iOriginFormat)
{
    if(iOriginFormat == FrameOriginFormat::kTopLeft) {
        return (y*_width + x);
    }

    return ((_height-y-1)*_width + x);
}
