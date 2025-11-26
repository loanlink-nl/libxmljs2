// Copyright 2009, Squish Tech, LLC.

#include <cstring>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "xml_syntax_error.h"

namespace {

void set_string_field(Napi::Env env, Napi::Object obj, const char *name,
                      const char *value) {
  if (!value) {
    return;
  }
  obj.Set(name, Napi::String::New(env, value, strlen(value)));
}

void set_numeric_field(Napi::Env env, Napi::Object obj, const char *name,
                       const int value) {
  obj.Set(name, Napi::Number::New(env, value));
}

char *xmlCharToChar(xmlChar *xmlStr) {
  char *charStr = (char *)malloc(xmlStrlen(xmlStr) + 1);
  if (charStr == NULL) {
    return NULL;
  }

  strncpy(charStr, (char *)xmlStr, xmlStrlen(xmlStr));
  charStr[xmlStrlen(xmlStr)] = '\0';

  return charStr;
}

} // anonymous namespace

namespace libxmljs {

Napi::Error XmlSyntaxError::BuildSyntaxError(Napi::Env env, xmlError *error) {
  Napi::Error err = Napi::Error::New(env, error->message);

  set_numeric_field(env, err.Value(), "domain", error->domain);
  set_numeric_field(env, err.Value(), "code", error->code);
  set_string_field(env, err.Value(), "message", error->message);
  set_numeric_field(env, err.Value(), "level", error->level);
  set_numeric_field(env, err.Value(), "column", error->int2);
  set_string_field(env, err.Value(), "file", error->file);
  set_numeric_field(env, err.Value(), "line", error->line);
  set_string_field(env, err.Value(), "str1", error->str1);
  set_string_field(env, err.Value(), "str2", error->str2);
  set_string_field(env, err.Value(), "str3", error->str3);

  if (error->node) {
    xmlNode *node = static_cast<xmlNode *>(error->node);
    xmlChar *nodePath = xmlGetNodePath(node);
    if (nodePath) {
      set_string_field(env, err.Value(), "xpath",
                       reinterpret_cast<const char *>(nodePath));
      xmlFree(nodePath);
    }
  }

  // only add if we have something interesting
  if (error->int1) {
    set_numeric_field(env, err.Value(), "int1", error->int1);
  }

  return err;
}

void XmlSyntaxError::PushToArray(void *errs, xmlError *error) {
  ErrorArrayContext *ctx = static_cast<ErrorArrayContext *>(errs);
  Napi::Env env = ctx->env;
  Napi::Array errors = ctx->errors;

  // Build the error object
  Napi::Error error_obj = XmlSyntaxError::BuildSyntaxError(env, error);

  // Push to array
  errors.Set(errors.Length(), error_obj.Value());
}

} // namespace libxmljs
