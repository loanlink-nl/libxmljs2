// Copyright 2009, Squish Tech, LLC.

#include <cstring>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "xml_syntax_error.h"

using namespace Napi;
namespace {

void set_string_field(Napi::Object obj, const char *name, const char *value) {
  Napi::HandleScope scope(env);
  if (!value) {
    return;
  }
  (obj).Set(Napi::String::New(env, name),
            Napi::String::New(env, value, strlen(value)));
}

void set_numeric_field(Napi::Object obj, const char *name, const int value) {
  Napi::HandleScope scope(env);
  (obj).Set(Napi::String::New(env, name), Napi::Int32::New(env, value));
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

Napi::Value XmlSyntaxError::BuildSyntaxError(xmlError *error) {
  Napi::EscapableHandleScope scope(env);

  Napi::Value err = Exception::Error(Napi::String::New(env, error->message));
  Napi::Object out = err.As<Napi::Object>();

  set_numeric_field(out, "domain", error->domain);
  set_numeric_field(out, "code", error->code);
  set_string_field(out, "message", error->message);
  set_numeric_field(out, "level", error->level);
  set_numeric_field(out, "column", error->int2);
  set_string_field(out, "file", error->file);
  set_numeric_field(out, "line", error->line);
  set_string_field(out, "str1", error->str1);
  set_string_field(out, "str2", error->str2);
  set_string_field(out, "str3", error->str3);

  if (error->node) {
    xmlNode *node = static_cast<xmlNode *>(error->node);
    char *xpath = xmlCharToChar(xmlGetNodePath(node));

    set_string_field(out, "xpath", xpath);

    free(xpath);
  }

  // only add if we have something interesting
  if (error->int1) {
    set_numeric_field(out, "int1", error->int1);
  }
  return scope.Escape(err);
}

void XmlSyntaxError::PushToArray(void *errs, xmlError *error) {
  Napi::HandleScope scope(env);
  Napi::Array errors = *reinterpret_cast<Napi::Array *>(errs);
  // push method for array
  Napi::Function push =
      Napi::Function::Cast((errors).Get(Napi::String::New(env, "push")));

  Napi::Value argv[1] = {XmlSyntaxError::BuildSyntaxError(error)};
  Napi::Call(push, errors, 1, argv);
}

} // namespace libxmljs
