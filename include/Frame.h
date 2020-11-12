#pragma once

#include <vector>

#include "types.h"

struct PixelColor
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;

    PixelColor(u32 col) {
        r = (col >> 24) & 0xFF;
        g = (col >> 16) & 0xFF;
        b = (col >>  8) & 0xFF;
        a = (col >>  0) & 0xFF;
    }
    PixelColor(u8 red, u8 green, u8 blue, u8 alpha)
        : r{red},
          g{green},
          b{blue},
          a{alpha}
    { }
};

enum class FrameOriginFormat { kTopLeft, kBottomLeft };

/* RBGA unsigned int8 type */
class Frame
{
public:
    Frame(int width, int height);
    Frame(int width, int height, const PixelColor& color);
    virtual ~Frame();

    const u8* data(FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
    int height() const { return _height; }
    PixelColor getBackdropColor();
    PixelColor getPixelColor(int x, int y);
    void setBackdropColor(const PixelColor& iColor);
    void setPixel(int x, int y, const PixelColor& iColor);
    void setPixel(int iPosIndex, const PixelColor& iColor);
    int width() const { return _width; }

private:
    PixelColor _backdropColor;
    int _height;
    std::vector<u8> _pixels;
    std::vector<u8> _pixelsBottom;
    int _width;

    static void SetPixel(std::vector<u8>& ioPixels, int iPixelIndex, const PixelColor& iColor);
    int pixelIndexToVectorIndex(int iPixelIndex, FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
    int posToPosIndex(int x, int y, FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
};

