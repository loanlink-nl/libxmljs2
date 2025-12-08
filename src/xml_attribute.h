// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ATTRIBUTE_H_
#define SRC_XML_ATTRIBUTE_H_

#include "libxmljs.h"
#include "xml_element.h"
#include "xml_namespace.h"

namespace libxmljs {

class XmlAttribute : public XmlNode<XmlAttribute> {
public:
  XmlAttribute(const Napi::CallbackInfo &info);

  static Napi::Function Init(Napi::Env env, Napi::Object exports);
  static Napi::FunctionReference constructor;

  static Napi::Value NewInstance(Napi::Env env, xmlNode *xml_obj,
                                 const xmlChar *name, const xmlChar *value);

  static Napi::Value NewInstance(Napi::Env env, xmlAttr *attr);

protected:
  Napi::Value Name(const Napi::CallbackInfo &info);
  Napi::Value AttrValue(const Napi::CallbackInfo &info);
  Napi::Value Node(const Napi::CallbackInfo &info);
  Napi::Value Namespace(const Napi::CallbackInfo &info);

  Napi::Value get_name(Napi::Env env);
  Napi::Value get_value(Napi::Env env);
  void set_value(const char *value);
  Napi::Value get_element(Napi::Env env);
  Napi::Value get_namespace(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_ATTRIBUTE_H_
