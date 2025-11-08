// Copyright 2009, Squish Tech, LLC.
#include "xml_attribute.h"
#include <cassert>

using namespace Napi;
namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

// XmlAttribute::XmlAttribute(const Napi::CallbackInfo &info) : XmlNode(info) {
//   Napi::HandleScope scope(env);
//
//   return info.This();
// }

// Napi::Object XmlAttribute::New(xmlNode *xml_obj, const xmlChar *name,
//                                const xmlChar *value) {
//   xmlAttr *attr = xmlSetProp(xml_obj, name, value);
//   assert(attr);
//
//   if (attr->_private) {
//     return scope.Escape(static_cast<XmlNode *>(xml_obj->_private)->handle());
//   }
//
//   XmlAttribute *attribute = new XmlAttribute(attr);
//   Napi::Object obj =
//       Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
//   attribute->Wrap(obj);
//   return scope.Escape(obj);
// }

// Napi::Object XmlAttribute::New(xmlAttr *attr) {
//   Napi::EscapableHandleScope scope(env);
//   assert(attr->type == XML_ATTRIBUTE_NODE);
//
//   if (attr->_private) {
//     return scope.Escape(static_cast<XmlNode *>(attr->_private)->handle());
//   }
//
//   XmlAttribute *attribute = new XmlAttribute(attr);
//   Napi::Object obj =
//       Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
//   attribute->Wrap(obj);
//   return scope.Escape(obj);
// }

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlAttribute *attr = info.This().Unwrap<XmlAttribute>();
  assert(attr);

  return attr->get_name();
}

Napi::Value XmlAttribute::Value(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlAttribute *attr = info.This().Unwrap<XmlAttribute>();
  assert(attr);

  // attr.value('new value');
  if (info.Length() > 0) {
    attr->set_value(info[0].As<Napi::String>().Utf8Value().c_str());
    return info.This();
  }

  // attr.value();
  return attr->get_value();
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlAttribute *attr = info.This().Unwrap<XmlAttribute>();
  assert(attr);

  return attr->get_element();
}

Napi::Value XmlAttribute::Namespace(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlAttribute *attr = info.This().Unwrap<XmlAttribute>();
  assert(attr);

  return attr->get_namespace();
}

Napi::Value XmlAttribute::get_name() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name,
                                          xmlStrlen(xml_obj->name)));

  return scope.Escape(env.Null());
}

Napi::Value XmlAttribute::get_value() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *value = xmlNodeGetContent(xml_obj);
  if (value != NULL) {
    Napi::String ret_value =
        Napi::String::New(env, (const char *)value, xmlStrlen(value));
    xmlFree(value);
    return scope.Escape(ret_value);
  }

  return scope.Escape(env.Null());
}

void XmlAttribute::set_value(const char *value) {
  if (xml_obj->children)
    xmlFreeNodeList(xml_obj->children);

  xml_obj->children = xml_obj->last = NULL;

  if (value) {
    xmlChar *buffer;
    xmlNode *tmp;

    // Encode our content
    buffer = xmlEncodeEntitiesReentrant(xml_obj->doc, (const xmlChar *)value);

    xml_obj->children = xmlStringGetNodeList(xml_obj->doc, buffer);
    xml_obj->last = NULL;
    tmp = xml_obj->children;

    // Loop through the children
    for (tmp = xml_obj->children; tmp; tmp = tmp->next) {
      tmp->parent = reinterpret_cast<xmlNode *>(xml_obj);
      tmp->doc = xml_obj->doc;
      if (tmp->next == NULL)
        xml_obj->last = tmp;
    }

    // Free up memory
    xmlFree(buffer);
  }
}

Napi::Value XmlAttribute::get_element() {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(XmlElement::New(xml_obj->parent));
}

Napi::Value XmlAttribute::get_namespace() {
  Napi::EscapableHandleScope scope(env);
  if (!xml_obj->ns) {
    return scope.Escape(env.Null());
  }
  return scope.Escape(XmlNamespace::New(xml_obj->ns));
}

void XmlAttribute::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference tmpl = Napi::Function::New(env, XmlAttribute::New);
  constructor.Reset(tmpl);
  tmpl->Inherit(Napi::New(env, XmlNode::constructor));

  Napi::SetPrototypeMethod(tmpl, "name", XmlAttribute::Name);
  Napi::SetPrototypeMethod(tmpl, "value", XmlAttribute::Value);
  Napi::SetPrototypeMethod(tmpl, "node", XmlAttribute::Node);
  Napi::SetPrototypeMethod(tmpl, "namespace", XmlAttribute::Namespace);

  (target).Set(Napi::String::New(env, "Attribute"), Napi::GetFunction(tmpl));
}

} // namespace libxmljs
