// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NAMESPACE_H_
#define SRC_XML_NAMESPACE_H_

#include <napi.h>
#include <uv.h>

#include "napi.h"
#include "uv.h"
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

protected:
  static Napi::Value New(const Napi::CallbackInfo &info);
  static Napi::Value Href(const Napi::CallbackInfo &info);
  static Napi::Value Prefix(const Napi::CallbackInfo &info);

  Napi::Value get_href();
  Napi::Value get_prefix();

  friend class Node;
};
} // namespace libxmljs

#endif // SRC_XML_NAMESPACE_H_
