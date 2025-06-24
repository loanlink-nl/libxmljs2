// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_SYNTAX_ERROR_H_
#define SRC_XML_SYNTAX_ERROR_H_

#include <libxml/xmlerror.h>

#include "libxmljs.h"

namespace libxmljs {

// basically being used like a namespace
class XmlSyntaxError {
public:
  // push xmlError onto v8::Array
  // helper method for xml library
  static void PushToArray(void *errs, xmlError *error);

  // create a N-API object for the syntax error
  // TODO make it a N-API Error object
  static Napi::Value BuildSyntaxError(Napi::Env env, xmlError *error);
};

} // namespace libxmljs

#endif // SRC_XML_SYNTAX_ERROR_H_
