// Copyright 2009, Squish Tech, LLC.

#include <napi.h>

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_xpath_context.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlElement::constructor;

// doc, name, content
Napi::Value XmlElement::New(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return info.This();
  }

  XmlDocument *document =
      Napi::ObjectWrap::Unwrap<XmlDocument>(info[0].To<Napi::Object>());
  assert(document);

  std::string name = info[1].As<Napi::String>();

  Napi::Value contentOpt;
  if (info[2].IsString()) {
    contentOpt = info[2];
  }
  std::string contentRaw = contentOpt.As<Napi::String>();
  const char *content = (contentRaw.Length()) ? *contentRaw : NULL;

  xmlChar *encodedContent =
      content
          ? xmlEncodeSpecialChars(document->xml_obj, (const xmlChar *)content)
          : NULL;
  xmlNode *elem = xmlNewDocNode(document->xml_obj, NULL, (const xmlChar *)*name,
                                encodedContent);
  if (encodedContent)
    xmlFree(encodedContent);

  XmlElement *element = new XmlElement(elem);
  elem->_private = element;
  element->Wrap(info.This());

  // this prevents the document from going away
  (info.This()).Set(Napi::String::New(env, "document"), info[0]).Check();

  return info.This();
}

Napi::Value XmlElement::Name(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  if (info.Length() == 0)
    return element->get_name();

  std::string name = info[0].As<Napi::String>(.To<Napi::String>());
  element->set_name(*name);
  return info.This();
}

Napi::Value XmlElement::Attr(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  // getter
  if (info.Length() == 1) {
    std::string name = info[0].As<Napi::String>();
    return element->get_attr(*name);
  }

  // setter
  std::string name = info[0].As<Napi::String>();
  std::string value = info[1].As<Napi::String>();
  element->set_attr(*name, *value);

  return info.This();
}

Napi::Value XmlElement::Attrs(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  return element->get_attrs();
}

Napi::Value XmlElement::AddChild(const Napi::CallbackInfo &info) {
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  XmlNode *child =
      Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());
  assert(child);

  xmlNode *imported_child = element->import_node(child->xml_obj);
  if (imported_child == NULL) {
    return Napi::ThrowError(
        "Could not add child. Failed to copy node to new Document.");
  }

  bool will_merge = element->child_will_merge(imported_child);
  if ((child->xml_obj == imported_child) && will_merge) {
    // merged child will be free, so ensure it is a copy
    imported_child = xmlCopyNode(imported_child, 0);
  }

  element->add_child(imported_child);

  if (!will_merge && (imported_child->_private != NULL)) {
    static_cast<XmlNode *>(imported_child->_private)->ref_wrapped_ancestor();
  }

  return info.This();
}

Napi::Value XmlElement::AddCData(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  Napi::Value contentOpt;
  if (info[0].IsString()) {
    contentOpt = info[0];
  }
  std::string contentRaw = contentOpt.As<Napi::String>();
  const char *content = (contentRaw.Length()) ? *contentRaw : NULL;

  xmlNode *elem =
      xmlNewCDataBlock(element->xml_obj->doc, (const xmlChar *)content,
                       xmlStrlen((const xmlChar *)content));

  element->add_cdata(elem);
  return info.This();
}

Napi::Value XmlElement::Find(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  std::string xpath = info[0].As<Napi::String>();

  XmlXpathContext ctxt(element->xml_obj);

  if (info.Length() == 2) {
    if (info[1].IsString()) {
      std::string uri = info[1].As<Napi::String>();
      ctxt.register_ns((const xmlChar *)"xmlns", (const xmlChar *)*uri);
    } else if (info[1].IsObject()) {
      Napi::Object namespaces = info[1].To<Napi::Object>();
      Napi::Array properties = Napi::GetPropertyNames(namespaces);
      for (unsigned int i = 0; i < properties->Length(); i++) {
        Napi::String prop_name =
            Napi::To<String>((properties).Get(Napi::Number::New(env, i)));
        std::string prefix = prop_name.As<Napi::String>();
        std::string uri = (namespaces).Get(prop_name.As<Napi::String>());
        ctxt.register_ns((const xmlChar *)*prefix, (const xmlChar *)*uri);
      }
    }
  }

  return ctxt.evaluate((const xmlChar *)*xpath);
}

Napi::Value XmlElement::NextElement(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  return element->get_next_element();
}

Napi::Value XmlElement::PrevElement(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  return element->get_prev_element();
}

Napi::Value XmlElement::Text(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  if (info.Length() == 0) {
    return element->get_content();
  } else {
    element->set_content(info[0].As<Napi::String>().Utf8Value().c_str());
  }

  return info.This();
}

Napi::Value XmlElement::Child(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  if (info.Length() != 1 || !info[0].IsNumber()) {
    Napi::Error::New(env, "Bad argument: must provide #child() with a number")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  const int32_t idx = info[0].As<Napi::Number>().Int32Value().ToChecked();
  return element->get_child(idx);
}

Napi::Value XmlElement::ChildNodes(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  if (info[0].IsNumber())
    return element->get_child(
        info[0].As<Napi::Number>().Int32Value().ToChecked());

  return element->get_child_nodes();
}

Napi::Value XmlElement::Path(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  return element->get_path();
}

Napi::Value XmlElement::AddPrevSibling(const Napi::CallbackInfo &info) {
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  XmlNode *new_sibling =
      Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    return Napi::ThrowError(
        "Could not add sibling. Failed to copy node to new Document.");
  }

  element->add_prev_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::AddNextSibling(const Napi::CallbackInfo &info) {
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  XmlNode *new_sibling =
      Napi::ObjectWrap::Unwrap<XmlNode>(info[0].To<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    return Napi::ThrowError(
        "Could not add sibling. Failed to copy node to new Document.");
  }

  element->add_next_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::Replace(const Napi::CallbackInfo &info) {
  XmlElement *element = info.This().Unwrap<XmlElement>();
  assert(element);

  if (info[0].IsString()) {
    element->replace_text(info[0].As<Napi::String>().Utf8Value().c_str());
  } else {
    XmlElement *new_sibling =
        Napi::ObjectWrap::Unwrap<XmlElement>(info[0].To<Napi::Object>());
    assert(new_sibling);

    xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
    if (imported_sibling == NULL) {
      return Napi::ThrowError(
          "Could not replace. Failed to copy node to new Document.");
    }
    element->replace_element(imported_sibling);
  }

  return info[0];
}

void XmlElement::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlElement::get_name() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name));
  else
    return scope.Escape(env.Undefined());
}

// TODO(sprsquish) make these work with namespaces
Napi::Value XmlElement::get_attr(const char *name) {
  Napi::EscapableHandleScope scope(env);
  xmlAttr *attr = xmlHasProp(xml_obj, (const xmlChar *)name);

  // why do we need a reference to the element here?
  if (attr) {
    return scope.Escape(XmlAttribute::New(attr));
  }

  return scope.Escape(env.Null());
}

// TODO(sprsquish) make these work with namespaces
void XmlElement::set_attr(const char *name, const char *value) {
  Napi::HandleScope scope(env);
  XmlAttribute::New(xml_obj, (const xmlChar *)name, (const xmlChar *)value);
}

Napi::Value XmlElement::get_attrs() {
  Napi::EscapableHandleScope scope(env);
  xmlAttr *attr = xml_obj->properties;

  if (!attr)
    return scope.Escape(Napi::Array::New(env, 0));

  Napi::Array attributes = Napi::Array::New(env);
  Napi::Function push =
      Napi::Function::Cast((attributes).Get(Napi::String::New(env, "push")));
  Napi::Value argv[1];
  do {
    argv[0] = XmlAttribute::New(attr);
    Napi::Call(push, attributes, 1, argv);
  } while ((attr = attr->next));

  return scope.Escape(attributes);
}

void XmlElement::add_cdata(xmlNode *cdata) { xmlAddChild(xml_obj, cdata); }

Napi::Value XmlElement::get_child(int32_t idx) {
  Napi::EscapableHandleScope scope(env);
  xmlNode *child = xml_obj->children;

  int32_t i = 0;
  while (child && i < idx) {
    child = child->next;
    ++i;
  }

  if (!child)
    return scope.Escape(env.Null());

  return scope.Escape(XmlNode::New(child));
}

Napi::Value XmlElement::get_child_nodes() {
  Napi::EscapableHandleScope scope(env);

  xmlNode *child = xml_obj->children;
  if (!child)
    return scope.Escape(Napi::Array::New(env, 0));

  uint32_t len = 0;
  do {
    ++len;
  } while ((child = child->next));

  Napi::Array children = Napi::Array::New(env, len);
  child = xml_obj->children;

  uint32_t i = 0;
  do {
    (children).Set(i, XmlNode::New(child));
  } while ((child = child->next) && ++i < len);

  return scope.Escape(children);
}

Napi::Value XmlElement::get_path() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return scope.Escape(js_obj);
}

void XmlElement::unlink_children() {
  xmlNode *cur = xml_obj->children;
  while (cur != NULL) {
    xmlNode *next = cur->next;
    if (cur->_private != NULL) {
      static_cast<XmlNode *>(cur->_private)->unref_wrapped_ancestor();
    }
    xmlUnlinkNode(cur);
    cur = next;
  }
}

void XmlElement::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  this->unlink_children();
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlElement::get_content() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Value XmlElement::get_next_element() {
  Napi::EscapableHandleScope scope(env);

  xmlNode *sibling = xml_obj->next;
  if (!sibling)
    return scope.Escape(env.Null());

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return scope.Escape(XmlElement::New(sibling));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlElement::get_prev_element() {
  Napi::EscapableHandleScope scope(env);

  xmlNode *sibling = xml_obj->prev;
  if (!sibling)
    return scope.Escape(env.Null());

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return scope.Escape(XmlElement::New(sibling));
  }

  return scope.Escape(env.Null());
}

Napi::Object XmlElement::New(xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->handle());
  }

  XmlElement *element = new XmlElement(node);
  Napi::Object obj =
      Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
  element->Wrap(obj);
  return scope.Escape(obj);
}

XmlElement::XmlElement(xmlNode *node) : XmlNode(node) {}

void XmlElement::replace_element(xmlNode *element) {
  xmlReplaceNode(xml_obj, element);
  if (element->_private != NULL) {
    XmlNode *node = static_cast<XmlNode *>(element->_private);
    node->ref_wrapped_ancestor();
  }
}

void XmlElement::replace_text(const char *content) {
  xmlNodePtr txt = xmlNewDocText(xml_obj->doc, (const xmlChar *)content);
  xmlReplaceNode(xml_obj, txt);
}

bool XmlElement::child_will_merge(xmlNode *child) {
  return ((child->type == XML_TEXT_NODE) && (xml_obj->last != NULL) &&
          (xml_obj->last->type == XML_TEXT_NODE) &&
          (xml_obj->last->name == child->name) && (xml_obj->last != child));
}

void XmlElement::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference tmpl = Napi::Function::New(env, New);
  constructor.Reset(tmpl);
  tmpl->Inherit(Napi::New(env, XmlNode::constructor));

  Napi::SetPrototypeMethod(tmpl, "addChild", XmlElement::AddChild);

  Napi::SetPrototypeMethod(tmpl, "cdata", XmlElement::AddCData);

  Napi::SetPrototypeMethod(tmpl, "_attr", XmlElement::Attr);

  Napi::SetPrototypeMethod(tmpl, "attrs", XmlElement::Attrs);

  Napi::SetPrototypeMethod(tmpl, "child", XmlElement::Child);

  Napi::SetPrototypeMethod(tmpl, "childNodes", XmlElement::ChildNodes);

  Napi::SetPrototypeMethod(tmpl, "find", XmlElement::Find);

  Napi::SetPrototypeMethod(tmpl, "nextElement", XmlElement::NextElement);

  Napi::SetPrototypeMethod(tmpl, "prevElement", XmlElement::PrevElement);

  Napi::SetPrototypeMethod(tmpl, "name", XmlElement::Name);

  Napi::SetPrototypeMethod(tmpl, "path", XmlElement::Path);

  Napi::SetPrototypeMethod(tmpl, "text", XmlElement::Text);

  Napi::SetPrototypeMethod(tmpl, "addPrevSibling", XmlElement::AddPrevSibling);

  Napi::SetPrototypeMethod(tmpl, "addNextSibling", XmlElement::AddNextSibling);

  Napi::SetPrototypeMethod(tmpl, "replace", XmlElement::Replace);

  (target).Set(Napi::String::New(env, "Element"), Napi::GetFunction(tmpl));
}

} // namespace libxmljs
