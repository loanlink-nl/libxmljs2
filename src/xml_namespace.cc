// Copyright 2009, Squish Tech, LLC.

#include <cassert>
#include <napi.h>

#include "xml_document.h"
#include "xml_namespace.h"
#include "xml_node.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlNamespace::constructor;

Napi::Value XmlNamespace::New(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);

  // created for an already existing namespace
  if (info.Length() == 0) {
    return info.This();
  }

  // TODO(sprsquish): ensure this is an actual Node object
  if (!info[0].IsObject())
    Napi::Error::New(env, "You must provide a node to attach this namespace to")
        .ThrowAsJavaScriptException();

  XmlNode node = *info[0].As<Napi::External<XmlNode>>().Data();

  std::string prefix;
  std::string href;

  if (info[1].IsString()) {
    prefix = info[1].ToString().Utf8Value();
  }

  href = info[2].ToString().Utf8Value();

  xmlNs *ns =
      xmlNewNs(node.xml_obj, (const xmlChar *)(href.c_str()),
               !prefix.empty() ? (const xmlChar *)(prefix.c_str()) : NULL);

  XmlNamespace *namesp = new XmlNamespace(info, ns);

  return info.This();
}

XmlNamespace::XmlNamespace(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlNamespace>(info) {
  auto env = info.Env();
  Napi::EscapableHandleScope scope(env);

  xml_obj = node;
  xml_obj->_private = this;

  /*
   * If a context is present and wrapped, increment its refcount to ensure
   * that it is considered accessible from javascript for as long as the
   * namespace is accessible.
   */
  if ((xml_obj->context) && (xml_obj->context->_private != NULL)) {
    this->context = xml_obj->context;
    // a namespace must be created on a given node
    XmlDocument *doc = static_cast<XmlDocument *>(xml_obj->context->_private);
    doc->Ref();
  } else {
    this->context = NULL;
  }
}

XmlNamespace::~XmlNamespace() {
  /*
   * `xml_obj` may have been nulled by `xmlDeregisterNodeCallback` when
   * the `xmlNs` was freed along with an attached node or document.
   */
  if (xml_obj != NULL) {
    xml_obj->_private = NULL;
  }

  /*
   * The context pointer is only set if this wrapper has incremented the
   * refcount of the context wrapper.
   */
  if (this->context != NULL) {
    if (this->context->_private != NULL) {
      // release the hold and allow the document to be freed
      XmlDocument *doc = static_cast<XmlDocument *>(this->context->_private);
      doc->Unref();
    }
    this->context = NULL;
  }

  // We do not free the xmlNode here. It could still be part of a document
  // It will be freed when the doc is freed
}

Napi::Value XmlNamespace::Href(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);
  XmlNamespace *ns = this;
  assert(ns);
  return ns->get_href(info);
}

Napi::Value XmlNamespace::Prefix(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);
  XmlNamespace *ns = this;
  assert(ns);
  return ns->get_prefix(info);
}

Napi::Value XmlNamespace::get_href(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->href)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->href,
                                          xmlStrlen(xml_obj->href)));

  return scope.Escape(env.Null());
}

Napi::Value XmlNamespace::get_prefix(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->prefix)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->prefix,
                                          xmlStrlen(xml_obj->prefix)));

  return scope.Escape(env.Null());
}

void XmlNamespace::Initialize(Napi::Object target) {
  auto env = target.Env();
  Napi::HandleScope scope(env);

  Napi::Function ctor =
      DefineClass(env, "XmlNamespace",
                  {
                      InstanceMethod<&XmlNamespace::Href>("href"),
                      InstanceMethod<&XmlNamespace::Prefix>("prefix"),
                  });

  constructor.Reset(ctor);

  constructor = Napi::Persistent(ctor);
  target.Set("XmlNamespace", ctor);
}
} // namespace libxmljs
