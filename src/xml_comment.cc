#include <napi.h>
#include <uv.h>

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_xpath_context.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlComment::constructor;

// doc, content
Napi::Value XmlComment::New(const Napi::CallbackInfo &info) {
  NAN_CONSTRUCTOR_CHECK(Comment)
  Napi::HandleScope scope(env);

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return return info.This();
  }

  DOCUMENT_ARG_CHECK

  XmlDocument *document = doc.Unwrap<XmlDocument>();
  assert(document);

  Napi::Value contentOpt;
  if (info[1].IsString()) {
    contentOpt = info[1];
  }
  std::string contentRaw = contentOpt.As<Napi::String>();
  const char *content = (contentRaw.Length()) ? *contentRaw : NULL;

  xmlNode *comm = xmlNewDocComment(document->xml_obj, (xmlChar *)content);

  XmlComment *comment = new XmlComment(comm);
  comm->_private = comment;
  comment->Wrap(info.This());

  // this prevents the document from going away
  (info.This()).Set(Napi::String::New(env, "document"), info[0]).Check();

  return return info.This();
}

Napi::Value XmlComment::Text(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlComment *comment = info.This().Unwrap<XmlComment>();
  assert(comment);

  if (info.Length() == 0) {
    return return comment->get_content();
  } else {
    comment->set_content(info[0].As<Napi::String>().Utf8Value().c_str());
  }

  return return info.This();
}

void XmlComment::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Napi::Value XmlComment::get_content() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Object XmlComment::New(xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->handle());
  }

  XmlComment *comment = new XmlComment(node);
  Napi::Object obj =
      Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
  comment->Wrap(obj);
  return scope.Escape(obj);
}

XmlComment::XmlComment(xmlNode *node) : XmlNode(node) {}

void XmlComment::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference t = Napi::Napi::FunctionReference::New(
      env, static_cast<NAN_METHOD((*))>(New));
  t->Inherit(Napi::New(env, XmlNode::constructor));

  constructor.Reset(t);

  Napi::SetPrototypeMethod(t, "text", XmlComment::Text);

  (target).Set(Napi::String::New(env, "Comment"), Napi::GetFunction(t));
}

} // namespace libxmljs
