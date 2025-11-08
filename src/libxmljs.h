// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_LIBXMLJS_H_
#define SRC_LIBXMLJS_H_

#include "napi.h"
#include <napi.h>

#define LIBXMLJS_ARGUMENT_TYPE_CHECK(arg, type, err)                           \
  if (!arg->type()) {                                                          \
    Napi::TypeError::New(env, err).ThrowAsJavaScriptException();               \
    return env.Null();                                                         \
  }

#define NAN_CONSTRUCTOR_CHECK(name)                                            \
  if (!info.IsConstructCall()) {                                               \
    Napi::ThrowTypeError("Class constructor " #name                            \
                         " cannot be invoked without 'new'");                  \
    return;                                                                    \
  }

#define DOCUMENT_ARG_CHECK                                                     \
  if (info.Length() == 0 || info[0].IsNullOrUndefined()) {                     \
    Napi::Error::New(env, "document argument required")                        \
        .ThrowAsJavaScriptException();                                         \
    return;                                                                    \
  }                                                                            \
  Napi::Object doc = info[0].To<Napi::Object>();                               \
  if (!XmlDocument::constructor.Get(Isolate::GetCurrent())                     \
           ->HasInstance(doc)) {                                               \
    Napi::Error::New(env, "document argument must be an instance of Document") \
        .ThrowAsJavaScriptException();                                         \
    return;                                                                    \
  }

namespace libxmljs {

#ifdef LIBXML_DEBUG_ENABLED
static const bool debugging = true;
#else
static const bool debugging = false;
#endif

// Ensure that libxml is properly initialised and destructed at shutdown
class LibXMLJS {
public:
  LibXMLJS();
  virtual ~LibXMLJS();

private:
  static LibXMLJS instance;
};

} // namespace libxmljs

#endif // SRC_LIBXMLJS_H_
