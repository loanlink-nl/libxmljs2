// Copyright 2009, Squish Tech, LLC.

#include <cassert>
#include <napi.h>

#include "xml_document.h"
#include "xml_namespace.h"
#include "xml_node.h"

namespace libxmljs {

Napi::FunctionReference XmlNamespace::constructor;

XmlNamespace::XmlNamespace(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlNamespace>(info) {
  Napi::Env env = info.Env();

  // created for an already existing namespace
  if (info.Length() == 0) {
    return;
  } else if (info.Length() > 0 && info[0].IsExternal()) {
    auto external = info[0].As<Napi::External<xmlNs>>();
    xmlNs *data = external.Data();

    // Copy or take ownership of the struct
    this->xml_obj = data;
  }
  // TODO(sprsquish): ensure this is an actual Node object
  else if (info[0].IsObject()) {
    const char *prefix = nullptr;
    std::string prefix_str;
    const char *href = nullptr;
    std::string href_str;

    if (info.Length() > 1 && info[1].IsString()) {
      prefix_str = info[1].ToString().Utf8Value();
      prefix = prefix_str.c_str();
    }

    if (info.Length() > 2) {
      href_str = info[2].ToString().Utf8Value();
      href = href_str.c_str();
    }

    auto node = XmlNodeInstance::Unwrap(info[0].ToObject());

    xmlNs *ns = xmlNewNs(node->xml_obj, (const xmlChar *)href,
                         prefix ? (const xmlChar *)prefix : NULL);

    xml_obj = ns;
  } else {
    Napi::Error::New(env, "You must provide a node to attach this namespace to")
        .ThrowAsJavaScriptException();
    return;
  }

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

Napi::Value XmlNamespace::NewInstance(Napi::Env env, xmlNs *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return scope.Escape(static_cast<XmlNamespace *>(node->_private)->Value());
  }

  auto external = Napi::External<xmlNs>::New(env, node);
  Napi::Object obj = constructor.New({external});
  return scope.Escape(obj);
}

Napi::Value XmlNamespace::Href(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_href(env));
}

Napi::Value XmlNamespace::Prefix(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_prefix(env));
}

Napi::Value XmlNamespace::get_href(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->href) {
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->href,
                                          xmlStrlen(xml_obj->href)));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlNamespace::get_prefix(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->prefix) {
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->prefix,
                                          xmlStrlen(xml_obj->prefix)));
  }

  return scope.Escape(env.Null());
}

void XmlNamespace::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func =
      DefineClass(env, "Namespace",
                  {
                      InstanceMethod("href", &XmlNamespace::Href),
                      InstanceMethod("prefix", &XmlNamespace::Prefix),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  env.AddCleanupHook([]() { constructor.Reset(); });

  exports.Set("Namespace", func);
}
} // namespace libxmljs
