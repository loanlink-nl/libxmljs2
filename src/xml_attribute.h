// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ATTRIBUTE_H_
#define SRC_XML_ATTRIBUTE_H_

#include "libxmljs.h"
#include "xml_element.h"
#include "xml_namespace.h"

namespace libxmljs {

class XmlAttribute : public XmlNode {
public:
  // explicit XmlAttribute(xmlAttr *node)
  //     : XmlNode(reinterpret_cast<xmlNode *>(node)) {}

  static void Initialize(Napi::Object target);
  static Napi::FunctionReference constructor;

  XmlAttribute(const Napi::CallbackInfo &info);

  static Napi::Object New(const Napi::CallbackInfo &info, xmlNode *xml_obj,
                          const xmlChar *name, const xmlChar *value);
  static Napi::Object New(const Napi::CallbackInfo &info, xmlAttr *attr);

protected:
  static Napi::Value Name(const Napi::CallbackInfo &info);
  static Napi::Value Value(const Napi::CallbackInfo &info);
  static Napi::Value Node(const Napi::CallbackInfo &info);
  static Napi::Value Namespace(const Napi::CallbackInfo &info);

  Napi::Value get_name();
  Napi::Value get_value();
  void set_value(const char *value);
  Napi::Value get_element();
  Napi::Value get_namespace();
};

} // namespace libxmljs

#endif // SRC_XML_ATTRIBUTE_H_
