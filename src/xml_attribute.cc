// Copyright 2009, Squish Tech, LLC.
#include "xml_attribute.h"
#include <cassert>

using namespace Napi;
namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

Napi::Value XmlAttribute::New(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  // This constructor is called from JavaScript, which we don't support
  // Attributes are created internally through New(env, xmlAttr*)
  return info.This();
}

Napi::Object XmlAttribute::New(Napi::Env env, xmlNode *xml_obj, const xmlChar *name,
                               const xmlChar *value) {
  Napi::EscapableHandleScope scope(env);
  xmlAttr *attr = xmlSetProp(xml_obj, name, value);
  assert(attr);

  // For now, always create a new wrapper even if one exists
  // TODO: Store JS object reference to return existing wrapper
  XmlAttribute *attribute = new XmlAttribute(attr);
  Napi::Object obj = constructor.New({});
  return scope.Escape(obj).ToObject();
}

Napi::Object XmlAttribute::New(Napi::Env env, xmlAttr *attr) {
  Napi::EscapableHandleScope scope(env);
  assert(attr->type == XML_ATTRIBUTE_NODE);

  // For now, always create a new wrapper even if one exists
  // TODO: Store JS object reference to return existing wrapper
  XmlAttribute *attribute = new XmlAttribute(attr);
  Napi::Object obj = constructor.New({});
  return scope.Escape(obj).ToObject();
}

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlAttribute *attr = static_cast<XmlAttribute *>(Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>()));
  assert(attr);

  return attr->get_name(env);
}

Napi::Value XmlAttribute::Value(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlAttribute *attr = static_cast<XmlAttribute *>(Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>()));
  assert(attr);

  // attr.value('new value');
  if (info.Length() > 0) {
    attr->set_value(info[0].As<Napi::String>().Utf8Value().c_str());
    return info.This();
  }

  // attr.value();
  return attr->get_value(env);
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlAttribute *attr = static_cast<XmlAttribute *>(Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>()));
  assert(attr);

  return attr->get_element(env);
}

Napi::Value XmlAttribute::Namespace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlAttribute *attr = static_cast<XmlAttribute *>(Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>()));
  assert(attr);

  return attr->get_namespace(env);
}

Napi::Value XmlAttribute::get_name(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name,
                                          xmlStrlen(xml_obj->name)));

  return scope.Escape(env.Null());
}

Napi::Value XmlAttribute::get_value(Napi::Env env) {
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

Napi::Value XmlAttribute::get_element(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(XmlElement::New(env, xml_obj->parent));
}

Napi::Value XmlAttribute::get_namespace(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (!xml_obj->ns) {
    return scope.Escape(env.Null());
  }
  return scope.Escape(XmlNamespace::New(env, xml_obj->ns));
}

void XmlAttribute::Initialize(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);
  
  Napi::Function func = Napi::Function::New<XmlAttribute::New>(env, "Attribute");
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  
  // Set up prototype methods
  Napi::Object proto = func.Get("prototype").As<Napi::Object>();
  proto.Set("name", Napi::Function::New<XmlAttribute::Name>(env));
  proto.Set("value", Napi::Function::New<XmlAttribute::Value>(env));
  proto.Set("node", Napi::Function::New<XmlAttribute::Node>(env));
  proto.Set("namespace", Napi::Function::New<XmlAttribute::Namespace>(env));
  
  target.Set("Attribute", func);
}

} // namespace libxmljs
