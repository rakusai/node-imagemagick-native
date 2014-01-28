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
  Magick::InitializeMagick(NULL);
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
    THROW_ERROR_EXCEPTION("convert()'s 2nd argument should be a callback");
    NanReturnUndefined();
  }

  Local<Object> obj = Local<Object>::Cast(args[0]);
  NanCallback *callback = new NanCallback(args[1].As<Function>());

  Local<Object> srcData = Local<Object>::Cast(obj->Get(NanSymbol("srcData")));
  if ( srcData->IsUndefined() || ! Buffer::HasInstance(srcData) ) {
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
  const char* resizeStyle = NULL;
  String::AsciiValue resizeStyleAsciiValue(resizeStyleValue->ToString());
  if (!resizeStyleValue->IsUndefined()) {
    resizeStyle = static_cast<const char *>(malloc(resizeStyleAsciiValue.length() + 1));
    strcpy((char *) resizeStyle, (char *) *resizeStyleAsciiValue);
  } else {
    resizeStyle = "aspectfill";
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
NAN_METHOD(Crop) {
  NanScope();
  Magick::InitializeMagick(NULL);
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if (args.Length() != 2) {
    THROW_ERROR_EXCEPTION("crop() requires one option argument and one callback argument!");
    NanReturnUndefined();
  }

  if (!args[0]->IsObject()) {
    THROW_ERROR_EXCEPTION("crop()'s 1st argument should be an object");
    NanReturnUndefined();
  }

  if (!args[1]->IsFunction()) {
    THROW_ERROR_EXCEPTION("crop()'s 2nd argument should be a callback");
    NanReturnUndefined();
  }

  Local<Object> obj = Local<Object>::Cast(args[0]);
  NanCallback *callback = new NanCallback(args[1].As<Function>());

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! Buffer::HasInstance(srcData) ) {
    THROW_ERROR_EXCEPTION("crop()'s 1st argument should have \"srcData\" key with a Buffer instance");
    NanReturnUndefined();
  }

  Local<Number> pWidth = Local<Number>::Cast( obj->Get( NanSymbol("width") ) );
  if ( pWidth->NumberValue() > 1 || pWidth->NumberValue() < 0) {
    THROW_ERROR_EXCEPTION("\"width\" should be Number with the value between 0 and 1");
    NanReturnUndefined();
  }

  Local<Number> pHeight = Local<Number>::Cast( obj->Get( NanSymbol("height") ) );
  if ( pHeight->NumberValue() > 1 || pHeight->NumberValue() < 0) {
    THROW_ERROR_EXCEPTION("\"height\" should be Number with the value between 0 and 1");
    NanReturnUndefined();
  }

  Local<Number> pTop = Local<Number>::Cast( obj->Get( NanSymbol("top") ) );
  if ( pTop->NumberValue() > 1 || pTop->NumberValue() < 0) {
    THROW_ERROR_EXCEPTION("\"top\" should be Number with the value between 0 and 1");
    NanReturnUndefined();
  }

  Local<Number> pLeft = Local<Number>::Cast( obj->Get( NanSymbol("left") ) );
  if ( pLeft->NumberValue() > 1 || pLeft->NumberValue() < 0) {
    THROW_ERROR_EXCEPTION("\"left\" should be Number with the value between 0 and 1");
    NanReturnUndefined();
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob(Buffer::Data(srcData), Buffer::Length(srcData));

  if (pTop->IsUndefined() && pLeft->IsUndefined() && pWidth->IsUndefined() && pHeight->IsUndefined()) {
    THROW_ERROR_EXCEPTION("At least one of the following params should be defined: width, height, top, left");
    NanReturnUndefined();
  }

  unsigned int quality = NanUInt32OptionValue(obj, NanSymbol("quality"), 0);

  Local<Object> fmt = Local<Object>::Cast(obj->Get(NanSymbol("format")));
  char *format = NULL;
  size_t format_cnt;
  if (!fmt->IsUndefined())
    format = NanCString(obj->Get(NanSymbol("format")), &format_cnt);

  NanAsyncQueueWorker(new CropWorker(callback, debug, srcBlob, pWidth->NumberValue(), pHeight->NumberValue(), pTop->NumberValue(), pLeft->NumberValue(), quality, format));
  NanReturnUndefined();
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:        required. Buffer with binary image data
//                  debug:          optional. 1 or 0
//              }
NAN_METHOD(Identify) {
  NanScope();
  Magick::InitializeMagick(NULL);
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if (args.Length() != 2) {
    THROW_ERROR_EXCEPTION("normalize() requires one option argument and one callback argument!");
    NanReturnUndefined();
  }

  if (!args[0]->IsObject()) {
    THROW_ERROR_EXCEPTION("normalize()'s 1st argument should be an object");
    NanReturnUndefined();
  }

  if (!args[1]->IsFunction()) {
    THROW_ERROR_EXCEPTION("normalize()'s 2nd argument should be a callback");
    NanReturnUndefined();
  }

  Local<Object> obj = Local<Object>::Cast(args[0]);
  NanCallback *callback = new NanCallback(args[1].As<Function>());

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! Buffer::HasInstance(srcData) ) {
    THROW_ERROR_EXCEPTION("normalize()'s 1st argument should have \"srcData\" key with a Buffer instance");
    NanReturnUndefined();
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob( Buffer::Data(srcData), Buffer::Length(srcData) );

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

  Local<Object> out = Object::New();

  out->Set(NanSymbol("width"), Integer::New(image.columns()));
  out->Set(NanSymbol("height"), Integer::New(image.rows()));
  out->Set(NanSymbol("depth"), Integer::New(image.depth()));
  out->Set(NanSymbol("format"), String::New(image.magick().c_str()));

  Local<Value> argv[] = {Local<Value>::New(Undefined()), out};
  callback->Call(2, argv);
  NanReturnUndefined();
}

// input
//   args[ 0 ]: options. required, object with following key,values
//              {
//                  srcData:        required. Buffer with binary image data
//                  debug:          optional. 1 or 0
//              }
NAN_METHOD(Normalize) {
  NanScope();
  Magick::InitializeMagick(NULL);
  MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);

  if (args.Length() != 2) {
    THROW_ERROR_EXCEPTION("normalize() requires one option argument and one callback argument!");
    NanReturnUndefined();
  }

  if (!args[0]->IsObject()) {
    THROW_ERROR_EXCEPTION("normalize()'s 1st argument should be an object");
    NanReturnUndefined();
  }

  if (!args[1]->IsFunction()) {
    THROW_ERROR_EXCEPTION("normalize()'s 2nd argument should be a callback");
    NanReturnUndefined();
  }

  Local<Object> obj = Local<Object>::Cast(args[0]);
  NanCallback *callback = new NanCallback(args[1].As<Function>());

  Local<Object> srcData = Local<Object>::Cast( obj->Get( NanSymbol("srcData") ) );
  if ( srcData->IsUndefined() || ! Buffer::HasInstance(srcData) ) {
    THROW_ERROR_EXCEPTION("normalize()'s 1st argument should have \"srcData\" key with a Buffer instance");
    NanReturnUndefined();
  }

  int debug = NanUInt32OptionValue(obj, NanSymbol("debug"), 0);
  if (debug) printf( "debug: on\n" );

  Magick::Blob srcBlob(Buffer::Data(srcData), Buffer::Length(srcData));

  NanAsyncQueueWorker(new NormalizeWorker(callback, debug, srcBlob));
  NanReturnUndefined();
}

void init(Handle<Object> target) {
  target->Set(NanSymbol("convert"), FunctionTemplate::New(Convert)->GetFunction());
  target->Set(NanSymbol("crop"), FunctionTemplate::New(Crop)->GetFunction());
  target->Set(NanSymbol("identify"), FunctionTemplate::New(Identify)->GetFunction());
  target->Set(NanSymbol("normalize"), FunctionTemplate::New(Normalize)->GetFunction());
}

// There is no semi-colon after NODE_MODULE as it's not a function (see node.h).
// see http://nodejs.org/api/addons.html
NODE_MODULE(imagemagick, init)
