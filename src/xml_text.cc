// Copyright 2009, Squish Tech, LLC.

#include <napi.h>
#include <uv.h>

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_text.h"
#include "xml_xpath_context.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlText::constructor;

Napi::Value XmlText::get_path() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return scope.Escape(js_obj);
}

// doc, name, content
Napi::Value XmlText::New(const Napi::CallbackInfo &info) {
  NAN_CONSTRUCTOR_CHECK(Text)
  Napi::HandleScope scope(env);

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return return info.This();
  }

  DOCUMENT_ARG_CHECK
  if (!info[1].IsString()) {
    Napi::Error::New(env, "content argument must be of type string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  XmlDocument *document = doc.Unwrap<XmlDocument>();
  assert(document);

  Napi::Value contentOpt;
  if (info[1].IsString()) {
    contentOpt = info[1];
  }
  std::string contentRaw = contentOpt.As<Napi::String>();
  const char *content = (contentRaw.Length()) ? *contentRaw : NULL;

  xmlNode *textNode =
      xmlNewDocText(document->xml_obj, (const xmlChar *)content);

  XmlText *element = new XmlText(textNode);
  textNode->_private = element;
  element->Wrap(info.This());

  // this prevents the document from going away
  (info.This()).Set(Napi::String::New(env, "document"), info[0]).Check();

  return return info.This();
}

Napi::Value XmlText::NextElement(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlText *element = info.This().Unwrap<XmlText>();
  assert(element);

  return return element->get_next_element();
}

Napi::Value XmlText::PrevElement(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlText *element = info.This().Unwrap<XmlText>();
  assert(element);

  return return element->get_prev_element();
}

Napi::Value XmlText::Text(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlText *element = info.This().Unwrap<XmlText>();
  assert(element);

  if (info.Length() == 0) {
    return return element->get_content();
  } else {
    element->set_content(info[0].As<Napi::String>().Utf8Value().c_str());
  }

  return return info.This();
}

Napi::Value XmlText::AddPrevSibling(const Napi::CallbackInfo &info) {
  XmlText *text = info.This().Unwrap<XmlText>();
  assert(text);

  XmlNode *new_sibling =
      Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = text->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    return Napi::ThrowError(
        "Could not add sibling. Failed to copy node to new Document.");
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             text->prev_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  text->add_prev_sibling(imported_sibling);

  return return info[0];
}

Napi::Value XmlText::AddNextSibling(const Napi::CallbackInfo &info) {
  XmlText *text = info.This().Unwrap<XmlText>();
  assert(text);

  XmlNode *new_sibling =
      Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = text->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    return Napi::ThrowError(
        "Could not add sibling. Failed to copy node to new Document.");
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             text->next_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  text->add_next_sibling(imported_sibling);

  return return info[0];
}

Napi::Value XmlText::Replace(const Napi::CallbackInfo &info) {
  XmlText *element = info.This().Unwrap<XmlText>();
  assert(element);

  if (info[0].IsString()) {
    element->replace_text(info[0].As<Napi::String>().Utf8Value().c_str());
  } else {
    XmlText *new_sibling =
        Napi::ObjectWrap::Unwrap<XmlText>(info[0].To<Napi::Object>());
    assert(new_sibling);

    xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
    if (imported_sibling == NULL) {
      return Napi::ThrowError(
          "Could not replace. Failed to copy node to new Document.");
    }
    element->replace_element(imported_sibling);
  }

  return return info[0];
}

Napi::Value XmlText::Path(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlText *text = info.This().Unwrap<XmlText>();
  assert(text);

  return return text->get_path();
}

Napi::Value XmlText::Name(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlText *text = info.This().Unwrap<XmlText>();
  assert(text);

  if (info.Length() == 0)
    return return text->get_name();
  return return info.This();
}

void XmlText::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlText::get_content() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Value XmlText::get_name() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name));
  else
    return scope.Escape(env.Undefined());
}

Napi::Value XmlText::get_next_element() {
  Napi::EscapableHandleScope scope(env);

  xmlNode *sibling = xml_obj->next;
  if (!sibling)
    return scope.Escape(env.Null());

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return scope.Escape(XmlText::New(sibling));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlText::get_prev_element() {
  Napi::EscapableHandleScope scope(env);

  xmlNode *sibling = xml_obj->prev;
  if (!sibling)
    return scope.Escape(env.Null());

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return scope.Escape(XmlText::New(sibling));
  }

  return scope.Escape(env.Null());
}

Napi::Object XmlText::New(xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->handle());
  }

  XmlText *text = new XmlText(node);
  Napi::Object obj =
      Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
  text->Wrap(obj);
  return scope.Escape(obj);
}

XmlText::XmlText(xmlNode *node) : XmlNode(node) {}

void XmlText::add_prev_sibling(xmlNode *element) {
  xmlAddPrevSibling(xml_obj, element);
}

void XmlText::add_next_sibling(xmlNode *element) {
  xmlAddNextSibling(xml_obj, element);
}

void XmlText::replace_element(xmlNode *element) {
  xmlReplaceNode(xml_obj, element);
}

void XmlText::replace_text(const char *content) {
  xmlNodePtr txt = xmlNewDocText(xml_obj->doc, (const xmlChar *)content);
  xmlReplaceNode(xml_obj, txt);
}

bool XmlText::next_sibling_will_merge(xmlNode *child) {
  return (child->type == XML_TEXT_NODE);
}

bool XmlText::prev_sibling_will_merge(xmlNode *child) {
  return (child->type == XML_TEXT_NODE);
}

void XmlText::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference tmpl = Napi::Function::New(env, New);

  constructor.Reset(tmpl);

  tmpl->Inherit(Napi::New(env, XmlNode::constructor));

  Napi::SetPrototypeMethod(tmpl, "nextElement", XmlText::NextElement);

  Napi::SetPrototypeMethod(tmpl, "prevElement", XmlText::PrevElement);

  Napi::SetPrototypeMethod(tmpl, "text", XmlText::Text);

  Napi::SetPrototypeMethod(tmpl, "replace", XmlText::Replace);

  Napi::SetPrototypeMethod(tmpl, "path", XmlText::Path);

  Napi::SetPrototypeMethod(tmpl, "name", XmlText::Name);

  Napi::SetPrototypeMethod(tmpl, "addPrevSibling", XmlText::AddPrevSibling);

  Napi::SetPrototypeMethod(tmpl, "addNextSibling", XmlText::AddNextSibling);

  (target).Set(Napi::String::New(env, "Text"), Napi::GetFunction(tmpl));
}

} // namespace libxmljs
