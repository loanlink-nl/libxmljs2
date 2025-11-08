// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NAMESPACE_H_
#define SRC_XML_NAMESPACE_H_

#include <napi.h>

#include "napi.h"
#include <libxml/tree.h>

namespace libxmljs {

class XmlNamespace : public Napi::ObjectWrap<XmlNamespace> {
public:
  xmlNs *xml_obj;

  xmlDoc *context; // reference-managed context

  static void Initialize(Napi::Object target);
  static Napi::FunctionReference constructor;

  explicit XmlNamespace(const Napi::CallbackInfo &info);
  XmlNamespace(xmlNs *node, const char *prefix, const char *href);
  ~XmlNamespace();
  static Napi::Object New(xmlNs *ns);

protected:
  Napi::Value New(const Napi::CallbackInfo &info);
  Napi::Value Href(const Napi::CallbackInfo &info);
  Napi::Value Prefix(const Napi::CallbackInfo &info);

  Napi::Value get_href(const Napi::CallbackInfo &info);
  Napi::Value get_prefix(const Napi::CallbackInfo &info);

  friend class Node;
};
} // namespace libxmljs

#endif // SRC_XML_NAMESPACE_H_
