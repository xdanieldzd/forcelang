/*
    forcelang - language override plugin
    Written 2018/2019 by xdaniel - github.com/xdanieldzd
*/

typedef union
{
    struct rgbaStruct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    } rgba;
    uint32_t packed;
} colorRgba;

extern const colorRgba colorBlackTransparent, colorWhite, colorGreen, colorNone;
extern SceDisplayFrameBuf frameBufParam;

void drawUpdate(const SceDisplayFrameBuf *pParam);
void drawRectangle(int x, int y, int width, int height, colorRgba color);
void drawCharacter(int x, int y, int zoom, char ch, colorRgba color);
void drawString(int x, int y, int zoom, colorRgba foreColor, colorRgba backColor, const char *format, ...);
