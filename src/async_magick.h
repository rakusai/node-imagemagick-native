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

class CropWorker:public NanAsyncWorker {
  public:
    CropWorker(NanCallback *callback):NanAsyncWorker(callback) {
    };
    ~CropWorker();
    void Execute();
    void HandleOKCallback();
};

class IdentifyWorker:public NanAsyncWorker {
  public:
    IdentifyWorker(NanCallback *callback):NanAsyncWorker(callback) {
    };
    ~IdentifyWorker();
    void Execute();
    void HandleOKCallback();
};

class NormalizeWorker:public NanAsyncWorker {
  public:
    NormalizeWorker(NanCallback *callback):NanAsyncWorker(callback) {
    };
    ~NormalizeWorker();
    void Execute();
    void HandleOKCallback();
};

class QuantizeColorsWorker:public NanAsyncWorker {
  public:
    QuantizeColorsWorker(NanCallback *callback):NanAsyncWorker(callback) {
    };
    ~QuantizeColorsWorker();
    void Execute();
    void HandleOKCallback();
};
