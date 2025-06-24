#include <cstring>
#include <cassert>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlComment::constructor;

Napi::Value XmlComment::New(const Napi::CallbackInfo& info) {
  return (new XmlComment(info))->Value();
}

Napi::Value XmlComment::Text(const Napi::CallbackInfo& info) {
  if (info.Length() == 0) {
    return this->get_content(info.Env());
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    this->set_content(content.c_str());
  }

  return info.This();
}

void XmlComment::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Napi::Value XmlComment::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return ret_content;
  }

  return Napi::String::New(env, "");
}

Napi::Object XmlComment::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value().As<Napi::Object>();
  }

  // Use XmlNode::New to create the wrapper, which will create the appropriate subclass
  return XmlNode::New(env, node).As<Napi::Object>();
}

XmlComment::XmlComment(const Napi::CallbackInfo& info) : XmlNode(info) {
  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return;
  }

  if (info.Length() < 1 || !info[0].IsObject()) {
    Napi::TypeError::New(info.Env(), "Must provide document as first argument").ThrowAsJavaScriptException();
    return;
  }

  XmlDocument *document = XmlDocument::Unwrap(info[0].As<Napi::Object>());
  assert(document);

  xmlNode *comm;
  if (info.Length() > 1 && info[1].IsString()) {
    std::string contentStr = info[1].As<Napi::String>().Utf8Value();
    comm = xmlNewDocComment(document->xml_obj, (xmlChar *)contentStr.c_str());
  } else {
    comm = xmlNewDocComment(document->xml_obj, NULL);
  }

  this->xml_obj = comm;
  comm->_private = this;

  // this prevents the document from going away
  info.This().As<Napi::Object>().Set("document", info[0]);
}

XmlComment::XmlComment(xmlNode *node) : XmlNode(node) {}

void XmlComment::Initialize(Napi::Env env, Napi::Object target) {
  // Since XmlComment inherits from XmlNode, we don't create a separate class
  // The methods will be set on XmlNode instances when they are XmlComment types
  // For now, just create a placeholder constructor
  Napi::Function func = Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
    return XmlComment::New(info);
  }, "Comment");
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  target.Set("Comment", func);
}

} // namespace libxmljs
