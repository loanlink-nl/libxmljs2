// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ATTRIBUTE_H_
#define SRC_XML_ATTRIBUTE_H_

#include "libxmljs.h"
#include "xml_element.h"
#include "xml_namespace.h"

namespace libxmljs {

class XmlAttribute : public XmlNode {
public:
  explicit XmlAttribute(xmlAttr *node)
      : XmlNode(reinterpret_cast<xmlNode *>(node)) {}
      
  // N-API constructor
  XmlAttribute(const Napi::CallbackInfo& info);

  static void Initialize(Napi::Env env, Napi::Object target);
  static Napi::FunctionReference constructor;

  static Napi::Object New(Napi::Env env, xmlNode *xml_obj, const xmlChar *name,
                                   const xmlChar *value);

  static Napi::Object New(Napi::Env env, xmlAttr *attr);

protected:
  Napi::Value New(const Napi::CallbackInfo& info);
  Napi::Value Name(const Napi::CallbackInfo& info);
  Napi::Value Value(const Napi::CallbackInfo& info);
  Napi::Value Node(const Napi::CallbackInfo& info);
  Napi::Value Namespace(const Napi::CallbackInfo& info);

  Napi::Value get_name(Napi::Env env);
  Napi::Value get_value(Napi::Env env);
  void set_value(const char *value);
  Napi::Value get_element(Napi::Env env);
  Napi::Value get_namespace(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_ATTRIBUTE_H_
