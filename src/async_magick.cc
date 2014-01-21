#include "async_magick.h"

ConvertWorker::ConvertWorker(NanCallback *callback, int debug, Magick::Blob srcBlob, unsigned int width, unsigned int height, unsigned int quality, const char *format, const char *resizeStyle):NanAsyncWorker(callback) {
  this->debug       = debug;
  this->srcBlob     = srcBlob;
  this->width       = width;
  this->height      = height;
  this->quality     = quality;
  this->format      = format;
  this->resizeStyle = resizeStyle;
};
ConvertWorker::~ConvertWorker() {
  if (format)
    delete[] format;
};
void ConvertWorker::Execute() {
  printf("starting async work");
  Magick::Image image;
  try {
    image.read(srcBlob);
  } catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    this->errmsg = message.c_str();
    return;
  } catch (...) {
    this->errmsg = "unhandled error";
    return;
  }

  if (format)
    image.magick(format);
  if (debug)
    printf( "format: %s\n", format );

  if (debug)
    printf("original width,height: %d, %d\n", (int) image.columns(), (int) image.rows());

  if (width || height) {
    if (!width)
      width  = image.columns();
    if (!height)
      height = image.rows();

    // do resize
    if ( strcmp( resizeStyle, "aspectfill" ) == 0 ) {
      // ^ : Fill Area Flag ('^' flag)
      // is not implemented in Magick++
      // and gravity: center, extent doesnt look like working as exptected
      // so we do it ourselves

      // keep aspect ratio, get the exact provided size, crop top/bottom or left/right if necessary
      double aspectratioExpected = (double)height / (double)width;
      double aspectratioOriginal = (double)image.rows() / (double)image.columns();
      unsigned int xoffset = 0;
      unsigned int yoffset = 0;
      unsigned int resizewidth;
      unsigned int resizeheight;

      if ( aspectratioExpected > aspectratioOriginal ) {
        // expected is taller
        resizewidth  = (unsigned int)( (double)height / (double)image.rows() * (double)image.columns() + 1. );
        resizeheight = height;
        xoffset      = (unsigned int)( (resizewidth - width) / 2. );
        yoffset      = 0;
      } else {
        // expected is wider
        resizewidth  = width;
        resizeheight = (unsigned int)( (double)width / (double)image.columns() * (double)image.rows() + 1. );
        xoffset      = 0;
        yoffset      = (unsigned int)( (resizeheight - height) / 2. );
      }

      if (debug)
        printf("resize to: %d, %d\n", resizewidth, resizeheight);
      Magick::Geometry resizeGeometry(resizewidth, resizeheight, 0, 0, 0, 0);
      image.resize(resizeGeometry);

      // limit canvas size to cropGeometry
      if (debug)
        printf( "crop to: %d, %d, %d, %d\n", width, height, xoffset, yoffset );
      Magick::Geometry cropGeometry( width, height, xoffset, yoffset, 0, 0 );

      Magick::Color transparent("white");
      if (format) {
        // make background transparent for PNG
        // JPEG background becomes black if set transparent here
        transparent.alpha(1.);
      }
      image.extent( cropGeometry, transparent );
    } else if (strcmp (resizeStyle, "aspectfit") == 0 ) {
      // keep aspect ratio, get the maximum image which fits inside specified size
      char geometryString[32];
      sprintf( geometryString, "%dx%d", width, height );
      if (debug)
        printf( "resize to: %s\n", geometryString );
      image.resize(geometryString);
    } else if (strcmp (resizeStyle, "fill") == 0) {
      // change aspect ratio and fill specified size
      char geometryString[32];
      sprintf( geometryString, "%dx%d!", width, height );
      if (debug)
        printf( "resize to: %s\n", geometryString );
      image.resize(geometryString);
    } else {
      this->errmsg = "resizeStyle not supported";
      return;
    }

    if (debug)
      printf( "resized to: %d, %d\n", (int)image.columns(), (int)image.rows() );
  }

  if (quality) {
    if (debug)
      printf("quality: %d\n", quality);
    image.quality(quality);
  }

  image.write(&dstBlob);
};
void ConvertWorker::HandleOKCallback() {
  NanScope();
  Local<v8::Value> retBuffer = NanNewBufferHandle((char*)dstBlob.data(), dstBlob.length());
  Local<Value> argv[] = {Local<Value>::New(Undefined()), retBuffer};
  callback->Call(2, argv);
};
