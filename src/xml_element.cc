// Copyright 2009, Squish Tech, LLC.

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlElement::constructor;

// doc, name, content
XmlElement::XmlElement(const Napi::CallbackInfo &info) : XmlNode(info) {
  Napi::Env env = info.Env();

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return;
  }

  XmlDocument *document =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].As<Napi::Object>());
  assert(document);

  std::string name = info[1].As<Napi::String>().Utf8Value();

  const char *content = NULL;
  std::string contentStr;
  if (info[2].IsString()) {
    contentStr = info[2].As<Napi::String>().Utf8Value();
    if (!contentStr.empty()) {
      content = contentStr.c_str();
    }
  }

  xmlChar *encodedContent =
      content
          ? xmlEncodeSpecialChars(document->xml_obj, (const xmlChar *)content)
          : NULL;
  xmlNode *elem = xmlNewDocNode(document->xml_obj, NULL,
                                (const xmlChar *)name.c_str(), encodedContent);
  if (encodedContent)
    xmlFree(encodedContent);

  XmlElement *element =
      XmlNode::Unwrap(XmlNode::NewInstance(env, elem).ToObject());
  elem->_private = element;

  // this prevents the document from going away
  info.This().As<Napi::Object>().Set("document", info[0]);

  return;
}

XmlElement::~XmlElement() {}

Napi::Value XmlElement::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value();
  }

  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = constructor.New({external});

  return scope.Escape(instance).ToObject();
}

Napi::Value XmlElement::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  if (info.Length() == 0)
    return element->get_name(env);

  std::string name = info[0].As<Napi::String>().Utf8Value();
  element->set_name(name.c_str());
  return info.This();
}

Napi::Value XmlElement::Attr(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  // getter
  if (info.Length() == 1) {
    std::string name = info[0].As<Napi::String>().Utf8Value();
    return element->get_attr(env, name.c_str());
  }

  // setter
  std::string name = info[0].As<Napi::String>().Utf8Value();
  std::string value = info[1].As<Napi::String>().Utf8Value();
  element->set_attr(name.c_str(), value.c_str());

  return info.This();
}

Napi::Value XmlElement::Attrs(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  return element->get_attrs(env);
}

Napi::Value XmlElement::AddChild(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  XmlNode *child =
      Napi::ObjectWrap<XmlNode>::Unwrap(info[0].As<Napi::Object>());
  assert(child);

  xmlNode *imported_child = element->import_node(child->xml_obj);
  if (imported_child == NULL) {
    Napi::Error::New(
        env, "Could not add child. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Undefined();
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
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  const char *content = NULL;
  std::string contentStr;
  if (info[0].IsString()) {
    contentStr = info[0].As<Napi::String>().Utf8Value();
    if (!contentStr.empty()) {
      content = contentStr.c_str();
    }
  }

  xmlNode *elem =
      xmlNewCDataBlock(element->xml_obj->doc, (const xmlChar *)content,
                       xmlStrlen((const xmlChar *)content));

  element->add_cdata(elem);
  return info.This();
}

Napi::Value XmlElement::Find(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  std::string xpath = info[0].As<Napi::String>().Utf8Value();

  XmlXpathContext ctxt(element->xml_obj);

  if (info.Length() == 2) {
    if (info[1].IsString()) {
      std::string uri = info[1].As<Napi::String>().Utf8Value();
      ctxt.register_ns((const xmlChar *)"xmlns", (const xmlChar *)uri.c_str());
    } else if (info[1].IsObject()) {
      Napi::Object namespaces = info[1].As<Napi::Object>();
      Napi::Array properties = namespaces.GetPropertyNames();
      for (unsigned int i = 0; i < properties.Length(); i++) {
        Napi::Value prop_name_val = properties.Get(i);
        std::string prop_name = prop_name_val.As<Napi::String>().Utf8Value();
        Napi::Value uri_val = namespaces.Get(prop_name);
        std::string uri = uri_val.As<Napi::String>().Utf8Value();
        ctxt.register_ns((const xmlChar *)prop_name.c_str(),
                         (const xmlChar *)uri.c_str());
      }
    }
  }

  return ctxt.evaluate(env, (const xmlChar *)xpath.c_str());
}

Napi::Value XmlElement::NextElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  return element->get_next_element(env);
}

Napi::Value XmlElement::PrevElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  return element->get_prev_element(env);
}

Napi::Value XmlElement::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  if (info.Length() == 0) {
    return element->get_content(env);
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    element->set_content(content.c_str());
  }

  return info.This();
}

Napi::Value XmlElement::Child(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  if (info.Length() != 1 || !info[0].IsNumber()) {
    Napi::Error::New(env, "Bad argument: must provide #child() with a number")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  const int32_t idx = info[0].As<Napi::Number>().Int32Value();
  return element->get_child(env, idx);
}

Napi::Value XmlElement::ChildNodes(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  if (info[0].IsNumber())
    return element->get_child(env, info[0].As<Napi::Number>().Int32Value());

  return element->get_child_nodes(env);
}

Napi::Value XmlElement::Path(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  return element->get_path(env);
}

Napi::Value XmlElement::AddPrevSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  XmlNode *new_sibling =
      Napi::ObjectWrap<XmlNode>::Unwrap(info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  element->add_prev_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::AddNextSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  XmlNode *new_sibling =
      Napi::ObjectWrap<XmlNode>::Unwrap(info[0].As<Napi::Object>());
  assert(new_sibling);

  xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  element->add_next_sibling(imported_sibling);

  if (imported_sibling->_private != NULL) {
    static_cast<XmlNode *>(imported_sibling->_private)->ref_wrapped_ancestor();
  }

  return info[0];
}

Napi::Value XmlElement::Replace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlElement *element = static_cast<XmlElement *>(node);
  assert(element);

  if (info[0].IsString()) {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    element->replace_text(content.c_str());
  } else {
    XmlNode *new_sibling_node =
        Napi::ObjectWrap<XmlNode>::Unwrap(info[0].As<Napi::Object>());
    XmlElement *new_sibling = static_cast<XmlElement *>(new_sibling_node);
    assert(new_sibling);

    xmlNode *imported_sibling = element->import_node(new_sibling->xml_obj);
    if (imported_sibling == NULL) {
      Napi::Error::New(
          env, "Could not replace. Failed to copy node to new Document.")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
    element->replace_element(imported_sibling);
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
    return XmlAttribute::NewInstance(env, attr);
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
    attributes.Set(i++, XmlAttribute::NewInstance(env, attr));
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

  return XmlNode::NewInstance(env, child);
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
    children.Set(i, XmlNode::NewInstance(env, child));
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
    return XmlElement::NewInstance(env, sibling);
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
    return XmlElement::NewInstance(env, sibling);
  }

  return env.Null();
}

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

Napi::Function XmlElement::GetClass(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "Element",
      {
          ObjectWrap<XmlElement>::StaticMethod("addChild",
                                               &XmlElement::AddChild),
          ObjectWrap<XmlElement>::StaticMethod("cdata", &XmlElement::AddCData),
          ObjectWrap<XmlElement>::StaticMethod("_attr", &XmlElement::Attr),
          ObjectWrap<XmlElement>::StaticMethod("attrs", &XmlElement::Attrs),
          ObjectWrap<XmlElement>::StaticMethod("child", &XmlElement::Child),
          ObjectWrap<XmlElement>::StaticMethod("childNodes",
                                               &XmlElement::ChildNodes),
          ObjectWrap<XmlElement>::StaticMethod("find", &XmlElement::Find),
          ObjectWrap<XmlElement>::StaticMethod("nextElement",
                                               &XmlElement::NextElement),
          ObjectWrap<XmlElement>::StaticMethod("prevElement",
                                               &XmlElement::PrevElement),
          ObjectWrap<XmlElement>::StaticMethod("name", &XmlElement::Name),
          ObjectWrap<XmlElement>::StaticMethod("path", &XmlElement::Path),
          ObjectWrap<XmlElement>::StaticMethod("text", &XmlElement::Text),
          ObjectWrap<XmlElement>::StaticMethod("addPrevSibling",
                                               &XmlElement::AddPrevSibling),
          ObjectWrap<XmlElement>::StaticMethod("addNextSibling",
                                               &XmlElement::AddNextSibling),
          ObjectWrap<XmlElement>::StaticMethod("replace", &XmlElement::Replace),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Element", func);

  return func;
}

} // namespace libxmljs
