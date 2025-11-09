// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NAMESPACE_H_
#define SRC_XML_NAMESPACE_H_

#include <libxml/tree.h>
#include <napi.h>

namespace libxmljs {

class XmlNamespace : public Napi::ObjectWrap<XmlNamespace> {
public:
  XmlNamespace(const Napi::CallbackInfo &info);
  ~XmlNamespace();

  xmlNs *xml_obj;
  xmlDoc *context; // reference-managed context

  static void Initialize(Napi::Env env, Napi::Object exports);
  static Napi::FunctionReference constructor;

  static Napi::Value NewInstance(Napi::Env env, xmlNs *ns);

protected:
  static Napi::Value Href(const Napi::CallbackInfo &info);
  static Napi::Value Prefix(const Napi::CallbackInfo &info);

  Napi::Value get_href(Napi::Env env);
  Napi::Value get_prefix(Napi::Env env);

  friend class Node;
};
} // namespace libxmljs

#endif // SRC_XML_NAMESPACE_H_
