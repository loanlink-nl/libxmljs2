#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlComment::constructor;

XmlComment::XmlComment(const Napi::CallbackInfo &info) : XmlNode(info) {
  Napi::Env env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Comment constructor must be called with new")
        .ThrowAsJavaScriptException();
    return;
  }

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return;
  }

  if (!info[0].IsObject()) {
    Napi::TypeError::New(env, "Document argument must be an object")
        .ThrowAsJavaScriptException();
    return;
  }

  Napi::Object docObj = info[0].As<Napi::Object>();
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(docObj);
  if (document == nullptr) {
    Napi::Error::New(env, "Invalid document argument")
        .ThrowAsJavaScriptException();
    return;
  }

  const char *content = nullptr;
  std::string contentStr;
  if (info.Length() > 1 && info[1].IsString()) {
    contentStr = info[1].As<Napi::String>().Utf8Value();
    content = contentStr.c_str();
  }

  xmlNode *comm = xmlNewDocComment(document->xml_obj, (xmlChar *)content);

  XmlComment *comment =
      XmlNode::Unwrap(XmlNode::NewInstance(env, comm).ToObject());
  comm->_private = comment;

  // this prevents the document from going away
  info.This().As<Napi::Object>().Set("document", info[0]);
}

Napi::Value XmlComment::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlComment *comment =
      Napi::ObjectWrap<XmlComment>::Unwrap(info.This().As<Napi::Object>());

  if (comment == nullptr) {
    Napi::Error::New(env, "Invalid XmlComment instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info.Length() == 0) {
    return comment->get_content(env);
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    comment->set_content(content.c_str());
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

Napi::Value XmlComment::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value();
  }

  Napi::Function cons = constructor.Value();
  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = cons.New({external});

  return scope.Escape(instance).ToObject();
}

Napi::Function XmlComment::GetClass(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "Comment",
                                    {
                                        ObjectWrap<XmlComment>::StaticMethod("text", &XmlComment::Text),
                                    });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Comment", func);

  return func;
}

} // namespace libxmljs
