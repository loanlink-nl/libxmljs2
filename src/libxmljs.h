// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_LIBXMLJS_H_
#define SRC_LIBXMLJS_H_

#include <napi.h>

#define LIBXMLJS_ARGUMENT_TYPE_CHECK(env, arg, type, err)                      \
  if (!arg.type()) {                                                           \
    Napi::TypeError::New(env, err).ThrowAsJavaScriptException();               \
    return env.Undefined();                                                    \
  }

#define NAPI_CONSTRUCTOR_CHECK(env, info, name)                               \
  if (!info.IsConstructCall()) {                                               \
    Napi::TypeError::New(env, "Class constructor " #name                       \
                        " cannot be invoked without 'new'").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                    \
  }

#define DOCUMENT_ARG_CHECK(env, info)                                         \
  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {       \
    Napi::Error::New(env, "document argument required").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                    \
  }                                                                            \
  Napi::Object doc = info[0].As<Napi::Object>();                               \
  if (!doc.InstanceOf(XmlDocument::constructor.Value())) {                     \
    Napi::Error::New(env, "document argument must be an instance of Document").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                    \
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
