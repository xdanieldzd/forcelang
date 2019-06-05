/*
    forcelang - language override plugin
    Written 2018/2019 by xdaniel - github.com/xdanieldzd
*/

#include "globals.h"

#include "font.h"

#define SETPIXELBLEND(_rx, _ry, color) \
{ \
    colorRgba *op = (colorRgba *)frameBufParam.base + (_ry * frameBufParam.pitch) + _rx; \
    colorRgba np = { .rgba.r = ((uint16_t)op->rgba.r + color.rgba.r) >> 1, ((uint16_t)op->rgba.g + color.rgba.g) >> 1, ((uint16_t)op->rgba.b + color.rgba.b) >> 1, ((uint16_t)op->rgba.a + color.rgba.a) >> 1 }; \
    *op = np; \
}

const colorRgba colorBlackTransparent = { .rgba.r = 0x00, .rgba.g = 0x00, .rgba.b = 0x00, .rgba.a = 0x80 };
const colorRgba colorWhite = { .packed = 0xFFFFFFFF };
const colorRgba colorGreen = { .rgba.r = 0xB0, .rgba.g = 0xFF, .rgba.b = 0xB0, .rgba.a = 0xFF };
const colorRgba colorNone = (colorRgba){ .packed = 0x00000000 };

SceDisplayFrameBuf frameBufParam;

void drawUpdate(const SceDisplayFrameBuf *pParam)
{
    memcpy(&frameBufParam, pParam, sizeof(SceDisplayFrameBuf));
}

void drawRectangleLP(int x, int y, int width, int height, colorRgba color)
{
    if (color.rgba.a == 0) return;

    if (color.rgba.a == 0xFF)
    {
        for (int ry = y; ry < y + height; ry++)
        {
            memset((uint32_t *)frameBufParam.base + (ry * frameBufParam.pitch) + x, color.packed, sizeof(uint32_t) * width);
        }
    }

    if (color.rgba.a < 0xFF)
    {
        for (int ry = y; ry < y + height; ry += 2)
        {
            for (int rx = x; rx < x + width; rx += 2)
            {
                SETPIXELBLEND((rx    ), (ry    ), color);
                SETPIXELBLEND((rx + 1), (ry    ), color);
                SETPIXELBLEND((rx    ), (ry + 1), color);
                SETPIXELBLEND((rx + 1), (ry + 1), color);
            }
        }
    }
}

void drawCharacter(int x, int y, int zoom, char ch, colorRgba color)
{
    uint32_t *fb = frameBufParam.base;
    uint8_t *bitmap = font_8x16_data[0];

    if (ch >= 0x00 && ch < 0x80)
    {
        bitmap = font_8x16_data[(int)ch];
    }

    for (int by = 0; by < (16 << zoom); by++)
    {
        for (int bx = 0; bx < (8 << zoom); bx++)
        {
            if (bitmap[by >> zoom] >> (bx >> zoom) & 0x01)
            {
                fb[((y + by) * frameBufParam.pitch) + x + bx] = color.packed;
            }
        }
    }
}

void drawStringInternal(int x, int y, int zoom, colorRgba foreColor, colorRgba backColor, const char *str)
{
    int width = 0;
    int height = 1;
    int lineLen = 0;

    int i = 0;
    while(str[i] != '\0')
    {
        if (str[i] == '\n')
        {
            if (lineLen > width) width = lineLen;
            lineLen = 0;
            height++;
        }
        else
        {
            lineLen += (font_8x16_widths[(int)str[i]] + 1);
        }
        i++;
    }
    if (lineLen > width) width = lineLen;
    width <<= zoom;

    int offset = 0;
    if (backColor.rgba.a != 0)
    {
        drawRectangleLP(x, y, width + 8, ((height << zoom) * 16) + 8 + (((height - 1) << zoom) * 4), backColor);
        offset = 4;
    }

    int sx = x + offset, sy = y + offset;

    i = 0;
    while(str[i] != '\0')
    {
        if (str[i] == '\n')
        {
            sx = x + offset;
            sy += (20 << zoom);
        }
        else
        {
            drawCharacter(sx, sy, zoom, str[i], foreColor);
            sx += ((font_8x16_widths[(int)str[i]] + 1) << zoom);
        }
        i++;
    }
}

void drawString(int x, int y, int zoom, colorRgba foreColor, colorRgba backColor, const char *format, ...)
{
    char buffer[256] = "";

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    drawStringInternal(x, y, zoom, foreColor, backColor, buffer);
}
