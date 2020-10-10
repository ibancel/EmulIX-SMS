#pragma once

#include <cstdint>
#include <vector>

struct PixelColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    PixelColor(uint32_t col) {
        r = (col >> 24) & 0xFF;
        g = (col >> 16) & 0xFF;
        b = (col >>  8) & 0xFF;
        a = (col >>  0) & 0xFF;
    }
    PixelColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
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

    const uint8_t* data(FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
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
    std::vector<uint8_t> _pixels;
    std::vector<uint8_t> _pixelsBottom;
    int _width;

    static void SetPixel(std::vector<uint8_t>& ioPixels, int iPixelIndex, const PixelColor& iColor);
    int pixelIndexToVectorIndex(int iPixelIndex, FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
    int posToPosIndex(int x, int y, FrameOriginFormat iOriginFormat = FrameOriginFormat::kTopLeft);
};

