// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_SYNTAX_ERROR_H_
#define SRC_XML_SYNTAX_ERROR_H_

#include <libxml/xmlerror.h>

#include "libxmljs.h"

namespace libxmljs {

struct ErrorArrayContext {
  Napi::Env env;
  Napi::Array errors;
};

// Utility class for creating syntax error objects
// Not an ObjectWrap - just a namespace-like utility class
class XmlSyntaxError {
public:
  // push xmlError onto Napi::Array
  // helper method for xml library
  static void PushToArray(void *errs, xmlError *error);

  // create a Napi::Value object for the syntax error
  // TODO make it a proper Error object
  static Napi::Error BuildSyntaxError(Napi::Env env, xmlError *error);
};

} // namespace libxmljs

#endif // SRC_XML_SYNTAX_ERROR_H_
