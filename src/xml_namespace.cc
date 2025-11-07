// Copyright 2009, Squish Tech, LLC.

#include <napi.h>
#include <uv.h>

#include "xml_document.h"
#include "xml_namespace.h"
#include "xml_node.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlNamespace::constructor;

Napi::Value XmlNamespace::New(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  // created for an already existing namespace
  if (info.Length() == 0) {
    return return info.This();
  }

  // TODO(sprsquish): ensure this is an actual Node object
  if (!info[0].IsObject())
    return Napi::ThrowError(
        "You must provide a node to attach this namespace to");

  XmlNode *node = Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());

  std::string *prefix = 0;
  std::string *href = 0;

  if (info[1].IsString()) {
    prefix = new std::string(info[1]);
  }

  href = new std::string(info[2]);

  xmlNs *ns = xmlNewNs(node->xml_obj, (const xmlChar *)(href->operator*()),
                       prefix ? (const xmlChar *)(prefix->operator*()) : NULL);

  delete prefix;
  delete href;

  XmlNamespace *namesp = new XmlNamespace(ns);
  namesp->Wrap(info.This());

  return return info.This();
}

Napi::Object XmlNamespace::New(Napi::Env env, xmlNs *node) {
  Napi::EscapableHandleScope scope(env);
  if (node->_private) {
    // Return existing wrapped object
    XmlNamespace *existingNs = static_cast<XmlNamespace *>(node->_private);
    return scope.Escape(existingNs->Value().ToObject());
  }

  XmlNamespace *ns = new XmlNamespace(node);
  Napi::Object obj = constructor.New({});
  return scope.Escape(obj).ToObject();
}

XmlNamespace::XmlNamespace(xmlNs *node) : xml_obj(node) {
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
  Napi::HandleScope scope(env);
  XmlNamespace *ns = info.This().Unwrap<XmlNamespace>();
  assert(ns);
  return return ns->get_href();
}

Napi::Value XmlNamespace::Prefix(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNamespace *ns = info.This().Unwrap<XmlNamespace>();
  assert(ns);
  return return ns->get_prefix();
}

Napi::Value XmlNamespace::get_href() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->href)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->href,
                                          xmlStrlen(xml_obj->href)));

  return scope.Escape(env.Null());
}

Napi::Value XmlNamespace::get_prefix() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->prefix)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->prefix,
                                          xmlStrlen(xml_obj->prefix)));

  return scope.Escape(env.Null());
}

void XmlNamespace::Initialize(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::Function func = Napi::Function::New(env, New);
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  Napi::Object proto = func.Get("prototype").As<Napi::Object>();
  proto.Set("href", Napi::Function::New(env, XmlNamespace::Href));
  proto.Set("prefix", Napi::Function::New(env, XmlNamespace::Prefix));

  target.Set("Namespace", func);
}
} // namespace libxmljs
