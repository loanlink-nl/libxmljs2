// Copyright 2009, Squish Tech, LLC.
#include "xml_attribute.h"
#include <cassert>

namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

XmlAttribute::XmlAttribute(const Napi::CallbackInfo& info) : XmlNode(info) {
  // N-API constructor implementation
}

Napi::Object XmlAttribute::New(Napi::Env env, xmlNode *xml_obj, const xmlChar *name,
                                const xmlChar *value) {
  xmlAttr *attr = xmlSetProp(xml_obj, name, value);
  assert(attr);

  if (attr->_private) {
    return static_cast<XmlNode *>(xml_obj->_private)->Value().As<Napi::Object>();
  }

  return XmlAttribute::New(env, attr);
}

Napi::Object XmlAttribute::New(Napi::Env env, xmlAttr *attr) {
  assert(attr->type == XML_ATTRIBUTE_NODE);

  if (attr->_private) {
    return static_cast<XmlNode *>(attr->_private)->Value().As<Napi::Object>();
  }

  // Use XmlNode::New to create the wrapper, which will create the appropriate subclass
  return XmlNode::New(env, reinterpret_cast<xmlNode*>(attr)).As<Napi::Object>();
}

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo& info) {
  return this->get_name(info.Env());
}

Napi::Value XmlAttribute::Value(const Napi::CallbackInfo& info) {
  // attr.value('new value');
  if (info.Length() > 0) {
    std::string value = info[0].As<Napi::String>().Utf8Value();
    this->set_value(value.c_str());
    return info.This();
  }

  // attr.value();
  return this->get_value(info.Env());
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo& info) {
  return this->get_element(info.Env());
}

Napi::Value XmlAttribute::Namespace(const Napi::CallbackInfo& info) {
  return this->get_namespace(info.Env());
}

Napi::Value XmlAttribute::get_name(Napi::Env env) {
  if (xml_obj->name)
    return Napi::String::New(env, (const char *)xml_obj->name, xmlStrlen(xml_obj->name));

  return env.Null();
}

Napi::Value XmlAttribute::get_value(Napi::Env env) {
  xmlChar *value = xmlNodeGetContent(xml_obj);
  if (value != NULL) {
    Napi::String ret_value = Napi::String::New(env, (const char *)value, xmlStrlen(value));
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
  return XmlElement::New(env, xml_obj->parent);
}

Napi::Value XmlAttribute::get_namespace(Napi::Env env) {
  if (!xml_obj->ns) {
    return env.Null();
  }
  return XmlNamespace::New(env, xml_obj->ns);
}

void XmlAttribute::Initialize(Napi::Env env, Napi::Object target) {
  // Since XmlAttribute inherits from XmlNode, we don't create a separate class
  // The methods will be set on XmlNode instances when they are XmlAttribute types
  // For now, just create a placeholder constructor
  Napi::Function func = Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
    return info.This();
  });
  
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  target.Set("Attribute", func);
}

} // namespace libxmljs
