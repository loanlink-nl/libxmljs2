// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NAMESPACE_H_
#define SRC_XML_NAMESPACE_H_

#include "libxmljs.h"
#include <libxml/tree.h>

namespace libxmljs {

class XmlNamespace : public Napi::ObjectWrap<XmlNamespace> {
public:
  xmlNs *xml_obj;

  xmlDoc *context; // reference-managed context

  static void Initialize(Napi::Env env, Napi::Object target);
  static Napi::FunctionReference constructor;

  explicit XmlNamespace(xmlNs *ns);
  XmlNamespace(xmlNs *node, const char *prefix, const char *href);
  ~XmlNamespace();

  static Napi::Object New(Napi::Env env, xmlNs *ns);
  
  // N-API constructor
  XmlNamespace(const Napi::CallbackInfo& info);

protected:
  Napi::Value Href(const Napi::CallbackInfo& info);
  Napi::Value Prefix(const Napi::CallbackInfo& info);

  Napi::Value get_href(Napi::Env env);
  Napi::Value get_prefix(Napi::Env env);

  friend class Node;
};
} // namespace libxmljs

#endif // SRC_XML_NAMESPACE_H_
