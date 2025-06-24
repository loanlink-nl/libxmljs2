// Copyright 2009, Squish Tech, LLC.

#include <cstring>
#include <cassert>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlElement::constructor;

Napi::Value XmlElement::New(const Napi::CallbackInfo& info) {
  return (new XmlElement(info))->Value();
}

XmlElement::XmlElement(const Napi::CallbackInfo& info) : XmlNode(info) {
  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return;
  }

  XmlDocument *document = XmlDocument::Unwrap(info[0].As<Napi::Object>());
  assert(document);

  std::string name = info[1].As<Napi::String>().Utf8Value();

  const char *content = NULL;
  if (info[2].IsString()) {
    std::string contentStr = info[2].As<Napi::String>().Utf8Value();
    content = contentStr.c_str();
  }

  xmlChar *encodedContent =
      content
          ? xmlEncodeSpecialChars(document->xml_obj, (const xmlChar *)content)
          : NULL;
  xmlNode *elem = xmlNewDocNode(document->xml_obj, NULL, (const xmlChar *)name.c_str(),
                                encodedContent);
  if (encodedContent)
    xmlFree(encodedContent);

  this->xml_obj = elem;
  elem->_private = this;

  // this prevents the document from going away
  info.This().As<Napi::Object>().Set("document", info[0]);
}

Napi::Value XmlElement::Name(const Napi::CallbackInfo& info) {
  if (info.Length() == 0)
    return this->get_name(info.Env());

  std::string name = info[0].As<Napi::String>().Utf8Value();
  this->set_name(name.c_str());
  return info.This();
}

Napi::Value XmlElement::Attr(const Napi::CallbackInfo& info) {
  // getter
  if (info.Length() == 1) {
    std::string name = info[0].As<Napi::String>().Utf8Value();
    return this->get_attr(info.Env(), name.c_str());
  }

  // setter
  std::string name = info[0].As<Napi::String>().Utf8Value();
  std::string value = info[1].As<Napi::String>().Utf8Value();
  this->set_attr(name.c_str(), value.c_str());

  return info.This();
}

Napi::Value XmlElement::Attrs(const Napi::CallbackInfo& info) {
  return this->get_attrs(info.Env());
}

Napi::Value XmlElement::AddChild(const Napi::CallbackInfo& info) {
  XmlNode *child = XmlNode::Unwrap(info[0].As<Napi::Object>());
  assert(child);

  xmlNode *imported_child = this->import_node(child->xml_obj);
  if (imported_child == NULL) {
    Napi::Error::New(info.Env(), "Could not add child. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  bool will_merge = this->child_will_merge(imported_child);
  if ((child->xml_obj == imported_child) && will_merge) {
    // merged child will be free, so ensure it is a copy
    imported_child = xmlCopyNode(imported_child, 0);
  }

  this->add_child(imported_child);

  if (!will_merge && (imported_child->_private != NULL)) {
    static_cast<XmlNode *>(imported_child->_private)->ref_wrapped_ancestor();
  }

  return info.This();
}

Napi::Value XmlElement::AddCData(const Napi::CallbackInfo& info) {
  const char *content = NULL;
  if (info[0].IsString()) {
    std::string contentStr = info[0].As<Napi::String>().Utf8Value();
    content = contentStr.c_str();
  }

  xmlNode *elem =
      xmlNewCDataBlock(this->xml_obj->doc, (const xmlChar *)content,
                       xmlStrlen((const xmlChar *)content));

  this->add_cdata(elem);
  return info.This();
}

Napi::Value XmlElement::Find(const Napi::CallbackInfo& info) {
  std::string xpath = info[0].As<Napi::String>().Utf8Value();

  XmlXpathContext ctxt(this->xml_obj);

  if (info.Length() == 2) {
    if (info[1].IsString()) {
      std::string uri = info[1].As<Napi::String>().Utf8Value();
      ctxt.register_ns((const xmlChar *)"xmlns", (const xmlChar *)uri.c_str());
    } else if (info[1].IsObject()) {
      Napi::Object namespaces = info[1].As<Napi::Object>();
      Napi::Array properties = namespaces.GetPropertyNames();
      for (uint32_t i = 0; i < properties.Length(); i++) {
        Napi::Value prop_name = properties.Get(i);
        std::string prefix = prop_name.As<Napi::String>().Utf8Value();
        std::string uri = namespaces.Get(prop_name).As<Napi::String>().Utf8Value();
        ctxt.register_ns((const xmlChar *)prefix.c_str(), (const xmlChar *)uri.c_str());
      }
    }
  }

  return ctxt.evaluate(info.Env(), (const xmlChar *)xpath.c_str());
}

Napi::Value XmlElement::NextElement(const Napi::CallbackInfo& info) {
  return this->get_next_element(info.Env());
}

Napi::Value XmlElement::PrevElement(const Napi::CallbackInfo& info) {
  return this->get_prev_element(info.Env());
}

Napi::Value XmlElement::Text(const Napi::CallbackInfo& info) {
  if (info.Length() == 0) {
    return this->get_content(info.Env());
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    this->set_content(content.c_str());
  }

  return info.This();
}

Napi::Value XmlElement::Child(const Napi::CallbackInfo& info) {
  if (info.Length() != 1 || !info[0].IsNumber()) {
    Napi::Error::New(info.Env(), "Bad argument: must provide #child() with a number")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  const int32_t idx = info[0].As<Napi::Number>().Int32Value();
  return this->get_child(info.Env(), idx);
}

Napi::Value XmlElement::ChildNodes(const Napi::CallbackInfo& info) {
  if (info[0].IsNumber())
    return this->get_child(info.Env(), info[0].As<Napi::Number>().Int32Value());

  return this->get_child_nodes(info.Env());
}

Napi::Value XmlElement::Path(const Napi::CallbackInfo& info) {
  return this->get_path(info.Env());
}

Napi::Value XmlElement::AddPrevSibling(const Napi::CallbackInfo& info) {
  XmlNode *new_sibling = XmlNode::Unwrap(info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(info.Env(), "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  this->add_prev_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::AddNextSibling(const Napi::CallbackInfo& info) {
  XmlNode *new_sibling = XmlNode::Unwrap(info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(info.Env(), "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  this->add_next_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::Replace(const Napi::CallbackInfo& info) {
  if (info[0].IsString()) {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    this->replace_text(content.c_str());
  } else {
    XmlNode *new_sibling = XmlNode::Unwrap(info[0].As<Napi::Object>());
    assert(new_sibling);

    xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
    if (imported_sibling == NULL) {
      Napi::Error::New(info.Env(), "Could not replace. Failed to copy node to new Document.")
          .ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }
    this->replace_element(imported_sibling);
  }

  return info[0];
}

void XmlElement::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlElement::get_name(Napi::Env env) {
  if (xml_obj->name)
    return Napi::String::New(env, (const char *)xml_obj->name);
  else
    return env.Undefined();
}

// TODO(sprsquish) make these work with namespaces
Napi::Value XmlElement::get_attr(Napi::Env env, const char *name) {
  xmlAttr *attr = xmlHasProp(xml_obj, (const xmlChar *)name);

  // why do we need a reference to the element here?
  if (attr) {
    return XmlAttribute::New(env, attr);
  }

  return env.Null();
}

// TODO(sprsquish) make these work with namespaces
void XmlElement::set_attr(const char *name, const char *value) {
  xmlSetProp(xml_obj, (const xmlChar *)name, (const xmlChar *)value);
}

Napi::Value XmlElement::get_attrs(Napi::Env env) {
  xmlAttr *attr = xml_obj->properties;

  if (!attr)
    return Napi::Array::New(env, 0);

  Napi::Array attributes = Napi::Array::New(env);
  uint32_t i = 0;
  do {
    attributes.Set(i++, XmlAttribute::New(env, attr));
  } while ((attr = attr->next));

  return attributes;
}

void XmlElement::add_cdata(xmlNode *cdata) { xmlAddChild(xml_obj, cdata); }

Napi::Value XmlElement::get_child(Napi::Env env, int32_t idx) {
  xmlNode *child = xml_obj->children;

  int32_t i = 0;
  while (child && i < idx) {
    child = child->next;
    ++i;
  }

  if (!child)
    return env.Null();

  return XmlNode::New(env, child);
}

Napi::Value XmlElement::get_child_nodes(Napi::Env env) {
  xmlNode *child = xml_obj->children;
  if (!child)
    return Napi::Array::New(env, 0);

  uint32_t len = 0;
  do {
    ++len;
  } while ((child = child->next));

  Napi::Array children = Napi::Array::New(env, len);
  child = xml_obj->children;

  uint32_t i = 0;
  do {
    children.Set(i, XmlNode::New(env, child));
  } while ((child = child->next) && ++i < len);

  return children;
}

Napi::Value XmlElement::get_path(Napi::Env env) {
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return js_obj;
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

Napi::Value XmlElement::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return ret_content;
  }

  return Napi::String::New(env, "");
}

Napi::Value XmlElement::get_next_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->next;
  if (!sibling)
    return env.Null();

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return XmlElement::New(env, sibling);
  }

  return env.Null();
}

Napi::Value XmlElement::get_prev_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->prev;
  if (!sibling)
    return env.Null();

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return XmlElement::New(env, sibling);
  }

  return env.Null();
}

Napi::Object XmlElement::New(Napi::Env env, xmlNode *node) {
  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value().As<Napi::Object>();
  }

  // Use XmlNode::New to create the wrapper, which will create the appropriate subclass
  return XmlNode::New(env, node).As<Napi::Object>();
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

void XmlElement::Initialize(Napi::Env env, Napi::Object target) {
  // Since XmlElement inherits from XmlNode, we don't create a separate class
  // The methods will be set on XmlNode instances when they are XmlElement types
  // For now, just create a placeholder constructor
  Napi::Function func = Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
    return XmlElement::New(info);
  }, "Element");
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  target.Set("Element", func);
}

} // namespace libxmljs
