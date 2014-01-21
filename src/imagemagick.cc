#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif  // BUILDING_NODE_EXTENSION

#include "imagemagick.h"
#include <list>
#include <string.h>
#include <exception>

#define THROW_ERROR_EXCEPTION(x) ThrowException(v8::Exception::Error(String::New(x))); \
  scope.Close(Undefined())

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:     required. Buffer with binary image data
//                  quality:     optional. 0-100 integer, default 75. JPEG/MIFF/PNG compression level.
//                  width:       optional. px.
//                  height:      optional. px.
//                  resizeStyle: optional. default: "aspectfill". can be "aspectfit", "fill"
//                  format:      optional. one of http://www.imagemagick.org/script/formats.php ex: "JPEG"
//                  debug:       optional. 1 or 0
//              }
//
NAN_METHOD(Convert) {
  NanScope();
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if (args.Length() != 2) {
    THROW_ERROR_EXCEPTION("convert() requires one option argument and one callback argument!");
    NanReturnUndefined();
  }

  if (!args[0]->IsObject()) {
    THROW_ERROR_EXCEPTION("convert()'s 1st argument should be an object");
    NanReturnUndefined();
  }

  if (!args[1]->IsFunction()) {
    THROW_ERROR_EXCEPTION("convert()'s 1st argument should be an object");
    NanReturnUndefined();
  }

  Local<Object> obj = Local<Object>::Cast(args[0]);
  NanCallback *callback = new NanCallback(args[1].As<Function>());

  Local<Object> srcData = Local<Object>::Cast(obj->Get(NanSymbol("srcData")));
  if ( srcData->IsUndefined() || ! node::Buffer::HasInstance(srcData) ) {
    THROW_ERROR_EXCEPTION("convert()'s 1st argument should have \"srcData\" key with a Buffer instance");
    NanReturnUndefined();
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob(Buffer::Data(srcData), Buffer::Length(srcData));

  unsigned int width = NanUInt32OptionValue(obj, NanSymbol("width"), 0);
  if (debug) printf( "width: %d\n", width );

  unsigned int height = NanUInt32OptionValue(obj, NanSymbol("height"), 0);
  if (debug) printf( "height: %d\n", height );

  Local<Value> resizeStyleValue = obj->Get(NanSymbol("resizeStyle"));
  const char* resizeStyle = "aspectfill";
  String::AsciiValue resizeStyleAsciiValue(resizeStyleValue->ToString());
  if (!resizeStyleValue->IsUndefined()) {
    resizeStyle = *resizeStyleAsciiValue;
  }
  if (debug) printf("resizeStyle: %s\n", resizeStyle);

  unsigned int quality = NanUInt32OptionValue(obj, NanSymbol("quality"), 0);

  Local<Object> fmt = Local<Object>::Cast(obj->Get(NanSymbol("format")));
  char *format = NULL;
  size_t format_cnt;
  if (!fmt->IsUndefined())
    format = NanCString(obj->Get(NanSymbol("format")), &format_cnt);

  NanAsyncQueueWorker(new ConvertWorker(callback, debug, srcBlob, width, height, quality, format, resizeStyle));
  NanReturnUndefined();
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:     required. Buffer with binary image data
//                  left:        required. 0-1 defines left corner crop position, default 0
//                  top:         required. 0-1 defines top corner crop position, default0
//                  quality:     optional. 0-100 integer, default 75. JPEG/MIFF/PNG compression level.
//                  width:       optional. 0-1 defines crop width, default is image.width
//                  height:      optional. 0-1 defines crop height, default is image.height
//                  format:      optional. one of http://www.imagemagick.org/script/formats.php ex: "JPEG"
//                  debug:       optional. 1 or 0
//              }
// TODO: convert into crop function
Handle<Value> Crop(const Arguments& args) {
  HandleScope scope;
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if ( args.Length() != 1 ) {
    return THROW_ERROR_EXCEPTION("crop() requires 1 (option) argument!");
  }
  if ( ! args[ 0 ]->IsObject() ) {
    return THROW_ERROR_EXCEPTION("crop()'s 1st argument should be an object");
  }
  Local<Object> obj = Local<Object>::Cast( args[ 0 ] );

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! node::Buffer::HasInstance(srcData) ) {
    return THROW_ERROR_EXCEPTION("crop()'s 1st argument should have \"srcData\" key with a Buffer instance");
  }

  Local<Number> pWidth = Local<Number>::Cast( obj->Get( NanSymbol("width") ) );
  if ( pWidth->NumberValue() > 1 || pWidth->NumberValue() < 0) {
    return THROW_ERROR_EXCEPTION("\"width\" should be Number with the value between 0 and 1");
  }

  Local<Number> pHeight = Local<Number>::Cast( obj->Get( NanSymbol("height") ) );
  if ( pHeight->NumberValue() > 1 || pHeight->NumberValue() < 0) {
    return THROW_ERROR_EXCEPTION("\"height\" should be Number with the value between 0 and 1");
  }

  Local<Number> pTop = Local<Number>::Cast( obj->Get( NanSymbol("top") ) );
  if ( pTop->NumberValue() > 1 || pTop->NumberValue() < 0) {
    return THROW_ERROR_EXCEPTION("\"top\" should be Number with the value between 0 and 1");
  }

  Local<Number> pLeft = Local<Number>::Cast( obj->Get( NanSymbol("left") ) );
  if ( pLeft->NumberValue() > 1 || pLeft->NumberValue() < 0) {
    return THROW_ERROR_EXCEPTION("\"left\" should be Number with the value between 0 and 1");
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );


  Magick::Blob srcBlob( node::Buffer::Data(srcData), node::Buffer::Length(srcData) );

  Magick::Image image;
  try {
    image.read( srcBlob );
  }
  catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    return THROW_ERROR_EXCEPTION(message.c_str());
  }
  catch (...) {
    return THROW_ERROR_EXCEPTION("unhandled error");
  }

  if (debug) printf("original width,height: %d, %d\n", (int) image.columns(), (int) image.rows());


  Local<Value> formatValue = obj->Get( NanSymbol("format") );
  String::AsciiValue format( formatValue->ToString() );
  if ( ! formatValue->IsUndefined() ) {
    if (debug) printf( "format: %s\n", *format );
    image.magick( *format );
  }

  if ( !(pTop->IsUndefined() && pLeft->IsUndefined() && pWidth->IsUndefined() && pHeight->IsUndefined()) ) {

    unsigned int width = pWidth->IsUndefined() ? image.columns():pWidth->NumberValue()*image.columns();
    if (debug) printf( "width: %d\n", width );

    unsigned int height = pHeight->IsUndefined() ? image.rows():pHeight->NumberValue()*image.rows();
    if (debug) printf( "height: %d\n", height );

    unsigned int top = pTop->IsUndefined() ? 0:pTop->NumberValue()*image.rows();
    if (debug) printf( "top: %d\n", top );

    unsigned int left = pLeft->IsUndefined() ? 0:pLeft->NumberValue()*image.columns();
    if (debug) printf( "left: %d\n", left );

    // limit canvas size to cropGeometry
    if (debug) printf("crop to: %d, %d, %d, %d\n", width, height, left, top);
    Magick::Geometry cropGeometry( width, height, left, top, 0, 0 );

    image.crop(cropGeometry);

    if (debug) printf( "cropped to: %d, %d\n", (int)image.columns(), (int)image.rows() );
  }

  //TODO remove quality settings and move them out into separate method
  unsigned int quality = NanUInt32OptionValue(obj, NanSymbol("quality"), 0);
  if ( quality ) {
    if (debug) printf( "quality: %d\n", quality );
    image.quality( quality );
  }

  Magick::Blob dstBlob;
  image.write( &dstBlob );

  node::Buffer* retBuffer = node::Buffer::New( dstBlob.length() );
  memcpy( node::Buffer::Data( retBuffer->handle_ ), dstBlob.data(), dstBlob.length() );
  return scope.Close( retBuffer->handle_ );
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:        required. Buffer with binary image data
//                  debug:          optional. 1 or 0
//              }
Handle<Value> Identify(const Arguments& args) {
  HandleScope scope;
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if ( args.Length() != 1 ) {
    return THROW_ERROR_EXCEPTION("identify() requires 1 (option) argument!");
  }
  Local<Object> obj = Local<Object>::Cast( args[ 0 ] );

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! node::Buffer::HasInstance(srcData) ) {
    return THROW_ERROR_EXCEPTION("identify()'s 1st argument should have \"srcData\" key with a Buffer instance");
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob( node::Buffer::Data(srcData), node::Buffer::Length(srcData) );

  Magick::Image image;
  try {
    image.read( srcBlob );
  }
  catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    return THROW_ERROR_EXCEPTION(message.c_str());
  }
  catch (...) {
    return THROW_ERROR_EXCEPTION("unhandled error");
  }

  if (debug) printf("original width,height: %d, %d\n", (int) image.columns(), (int) image.rows());

  Handle<Object> out = Object::New();

  out->Set(NanSymbol("width"), Integer::New(image.columns()));
  out->Set(NanSymbol("height"), Integer::New(image.rows()));
  out->Set(NanSymbol("depth"), Integer::New(image.depth()));
  out->Set(NanSymbol("format"), String::New(image.magick().c_str()));

  return scope.Close( out );
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:        required. Buffer with binary image data
//                  debug:          optional. 1 or 0
//              }
Handle<Value> Normalize(const Arguments& args) {
  HandleScope scope;
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if ( args.Length() != 1 ) {
    return THROW_ERROR_EXCEPTION("Normalize() requires 1 (option) argument!");
  }
  Local<Object> obj = Local<Object>::Cast( args[ 0 ] );

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! node::Buffer::HasInstance(srcData) ) {
    return THROW_ERROR_EXCEPTION("Normalize()'s 1st argument should have \"srcData\" key with a Buffer instance");
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob( node::Buffer::Data(srcData), node::Buffer::Length(srcData) );

  Magick::Image image;
  try {
    image.read( srcBlob );
  }
  catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    return THROW_ERROR_EXCEPTION(message.c_str());
  }
  catch (...) {
    return THROW_ERROR_EXCEPTION("unhandled error");
  }

  int orientation = atoi(image.attribute("EXIF:Orientation").c_str());
  if (debug) printf("orientation: %d\n", orientation);

  // Magick::DrawableAffine affine;
  switch (orientation) {
    case 1:
      // no need to do anything
      break;
    case 2:
      image.flip();
      break;
    case 3:
      image.rotate(180);
      break;
    case 4:
      image.flop();
      break;
    case 5:
      image.rotate(90);
      image.flip();
      break;
    case 6:
      image.rotate(90);
      break;
    case 7:
      image.rotate(-90);
      image.flip();
      break;
    case 8:
      image.rotate(-90);
      break;
    default:
      if (debug) printf("orientation is missing. skipping");
      return scope.Close(Undefined());
  }
  // image.affineTransform(affine);
  image.strip();

  Magick::Blob dstBlob;
  image.write( &dstBlob );

  node::Buffer* retBuffer = node::Buffer::New( dstBlob.length() );
  memcpy( node::Buffer::Data( retBuffer->handle_ ), dstBlob.data(), dstBlob.length() );
  return scope.Close( retBuffer->handle_ );
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:        required. Buffer with binary image data
//                  colors:         optional. 5 by default
//                  debug:          optional. 1 or 0
//              }
Handle<Value> QuantizeColors(const Arguments& args) {
  HandleScope scope;
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if ( args.Length() != 1 ) {
    return THROW_ERROR_EXCEPTION("quantizeColors() requires 1 (option) argument!");
  }
  Local<Object> obj = Local<Object>::Cast( args[ 0 ] );

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! node::Buffer::HasInstance(srcData) ) {
    return THROW_ERROR_EXCEPTION("quantizeColors()'s 1st argument should have \"srcData\" key with a Buffer instance");
  }

  int colorsCount = NanUInt32OptionValue(obj, NanSymbol("colors"), 0);
  if (!colorsCount) colorsCount = 5;

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob( node::Buffer::Data(srcData), node::Buffer::Length(srcData) );

  Magick::Image image;
  try {
    image.read( srcBlob );
  }
  catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    return THROW_ERROR_EXCEPTION(message.c_str());
  }
  catch (...) {
    return THROW_ERROR_EXCEPTION("unhandled error");
  }

  ssize_t rows = 196; ssize_t columns = 196;

  if (debug) printf( "resize to: %d, %d\n", (int) rows, (int) columns );
  Magick::Geometry resizeGeometry( rows, columns, 0, 0, 0, 0 );
  image.resize( resizeGeometry );

  if (debug) printf("totalColors before: %d\n", (int) image.totalColors());

  image.quantizeColors(colorsCount + 1);
  image.quantize();

  if (debug) printf("totalColors after: %d\n", (int) image.totalColors());

  Magick::PixelPacket* pixels = image.getPixels(0, 0, image.columns(), image.rows());

  Magick::PixelPacket* colors = new Magick::PixelPacket[colorsCount]();
  int index = 0;

  for ( ssize_t x = 0; x < rows ; x++ ) {
    for ( ssize_t y = 0; y < columns ; y++ ) {
      Magick::PixelPacket pixel = pixels[rows * x + y];

      bool found = false;
      for(int x = 0; x < colorsCount; x++)
        if (pixel.red == colors[x].red && pixel.green == colors[x].green && pixel.blue == colors[x].blue) found = true;

      if (!found) colors[index++] = pixel;
      if (index >= colorsCount) break;
    }
    if (index >= colorsCount) break;
  }

  Handle<Object> out = Array::New();

  for(int x = 0; x < colorsCount; x++)
    if (debug) printf("found rgb : %d %d %d\n", ((int) colors[x].red) / 255, ((int) colors[x].green) / 255, ((int) colors[x].blue) / 255);

  for(int x = 0; x < colorsCount; x++) {
    Local<Object> color = Object::New();

    int r = ((int) colors[x].red) / 255;
    if (r > 255) r = 255;

    int g = ((int) colors[x].green) / 255;
    if (g > 255) g = 255;

    int b = ((int) colors[x].blue) / 255;
    if (b > 255) b = 255;

    color->Set(NanSymbol("r"), Integer::New(r));
    color->Set(NanSymbol("g"), Integer::New(g));
    color->Set(NanSymbol("b"), Integer::New(b));

    char hexcol[16];
    snprintf(hexcol, sizeof hexcol, "%02x%02x%02x", r, g, b);
    color->Set(NanSymbol("hex"), String::New(hexcol));

    out->Set(x, color);
  }

  delete[] colors;

  return scope.Close( out );
}

void init(Handle<Object> target) {
  target->Set(NanSymbol("convert"), FunctionTemplate::New(Convert)->GetFunction());
  target->Set(NanSymbol("crop"), FunctionTemplate::New(Crop)->GetFunction());
  target->Set(NanSymbol("identify"), FunctionTemplate::New(Identify)->GetFunction());
  target->Set(NanSymbol("normalize"), FunctionTemplate::New(Normalize)->GetFunction());
  target->Set(NanSymbol("quantizeColors"), FunctionTemplate::New(QuantizeColors)->GetFunction());
}

// There is no semi-colon after NODE_MODULE as it's not a function (see node.h).
// see http://nodejs.org/api/addons.html
NODE_MODULE(imagemagick, init)
