// Copyright 2009, Squish Tech, LLC.
#include "xml_attribute.h"

namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

XmlAttribute::XmlAttribute(const Napi::CallbackInfo &info) : XmlNode(info) {}

XmlAttribute::~XmlAttribute() {
}

Napi::Value XmlAttribute::NewInstance(Napi::Env env, xmlNode *xml_obj,
                                      const xmlChar *name,
                                      const xmlChar *value) {
  Napi::EscapableHandleScope scope(env);

  xmlAttr *attr = xmlSetProp(xml_obj, name, value);
  assert(attr);

  if (attr->_private) {
    return static_cast<XmlNode *>(xml_obj->_private)->Value();
  }

  auto external = Napi::External<xmlAttr>::New(env, attr);
  Napi::Object obj = constructor.New({external});
  return scope.Escape(obj).ToObject();
}

Napi::Value XmlAttribute::NewInstance(Napi::Env env, xmlAttr *attr) {
  Napi::EscapableHandleScope scope(env);
  assert(attr->type == XML_ATTRIBUTE_NODE);

  if (attr->_private) {
    return static_cast<XmlNode *>(attr->_private)->Value();
  }

  auto external = Napi::External<xmlAttr>::New(env, attr);
  Napi::Object obj = constructor.New({external});
  return scope.Escape(obj).ToObject();
}

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlAttribute *attr = static_cast<XmlAttribute *>(node);
  assert(attr);

  return attr->get_name(env);
}

Napi::Value XmlAttribute::Value(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlAttribute *attr = static_cast<XmlAttribute *>(node);
  assert(attr);

  // attr.value('new value');
  if (info.Length() > 0) {
    std::string value_str = info[0].As<Napi::String>().Utf8Value();
    attr->set_value(value_str.c_str());
    return info.This();
  }

  // attr.value();
  return attr->get_value(env);
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlAttribute *attr = static_cast<XmlAttribute *>(node);
  assert(attr);

  return attr->get_element(env);
}

Napi::Value XmlAttribute::Namespace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlNode *node =
      Napi::ObjectWrap<XmlNode>::Unwrap(info.This().As<Napi::Object>());
  XmlAttribute *attr = static_cast<XmlAttribute *>(node);
  assert(attr);

  return attr->get_namespace(env);
}

Napi::Value XmlAttribute::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, (const char *)xml_obj->name,
                             xmlStrlen(xml_obj->name));
  }

  return env.Null();
}

Napi::Value XmlAttribute::get_value(Napi::Env env) {
  xmlChar *value = xmlNodeGetContent(xml_obj);
  if (value != NULL) {
    Napi::String ret_value =
        Napi::String::New(env, (const char *)value, xmlStrlen(value));
    xmlFree(value);
    return ret_value;
  }

  return env.Null();
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
  return XmlElement::NewInstance(env, xml_obj->parent);
}

Napi::Value XmlAttribute::get_namespace(Napi::Env env) {
  if (!xml_obj->ns) {
    return env.Null();
  }
  return XmlNamespace::NewInstance(env, xml_obj->ns);
}

static Napi::Value
AttributeConstructorCallback(const Napi::CallbackInfo &info) {
  // This is only called when constructing new instances from C++
  // The actual wrapping happens in XmlAttribute::NewInstance
  return info.This();
}

Napi::Function XmlAttribute::GetClass(Napi::Env env, Napi::Object exports) {
  Napi::Function func =
      Napi::Function::New(env, AttributeConstructorCallback, "Attribute");

  // Add instance methods to the prototype
  Napi::Object proto = func.Get("prototype").As<Napi::Object>();
  proto.Set("name", Napi::Function::New(env, XmlAttribute::Name));
  proto.Set("value", Napi::Function::New(env, XmlAttribute::Value));
  proto.Set("node", Napi::Function::New(env, XmlAttribute::Node));
  proto.Set("namespace", Napi::Function::New(env, XmlAttribute::Namespace));

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Attribute", func);

  return func;
}

} // namespace libxmljs
