#include <Magick++.h>
#include "nan.h"
using namespace node;
using namespace v8;

class ConvertWorker:public NanAsyncWorker {
  public:
    ConvertWorker(NanCallback *callback, int debug, Magick::Blob srcBlob, unsigned int width, unsigned int height, unsigned int quality, const char *format, const char *resizeStyle);
    ~ConvertWorker();
    void Execute();
    void HandleOKCallback();
  private:
    int debug;
    Magick::Blob srcBlob;
    Magick::Blob dstBlob;
    unsigned int width;
    unsigned int height;
    unsigned int quality;
    const char *format;
    const char *resizeStyle;
};

class ConvertFileWorker:public NanAsyncWorker {
  public:
    ConvertFileWorker(NanCallback *callback, int debug, const char *srcPath, const char *outPath, unsigned int width, unsigned int height, unsigned int quality, const char *format, const char *resizeStyle);
    ~ConvertFileWorker();
    void Execute();
    void HandleOKCallback();
  private:
    int debug;
    const char *srcPath;
    const char *outPath;
    unsigned int width;
    unsigned int height;
    unsigned int quality;
    const char *format;
    const char *resizeStyle;
};

class CropWorker:public NanAsyncWorker {
  public:
    CropWorker(NanCallback *callback, int debug, Magick::Blob srcBlob, double pWidth, double pHeight, double pTop, double pLeft, unsigned int quality, const char *format);
    ~CropWorker();
    void Execute();
    void HandleOKCallback();
  private:
    int debug;
    Magick::Blob srcBlob;
    Magick::Blob dstBlob;
    double pWidth;
    double pHeight;
    double pTop;
    double pLeft;
    unsigned int quality;
    const char *format;
};

class NormalizeWorker:public NanAsyncWorker {
  public:
    NormalizeWorker(NanCallback *callback, int debug, Magick::Blob srcBlob);
    ~NormalizeWorker();
    void Execute();
    void HandleOKCallback();
  private:
    int debug;
    Magick::Blob srcBlob;
    Magick::Blob dstBlob;
};

class QuantizeColorsWorker:public NanAsyncWorker {
  public:
    QuantizeColorsWorker(NanCallback *callback);
    ~QuantizeColorsWorker();
    void Execute();
    void HandleOKCallback();
};
