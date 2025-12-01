// Copyright 2009, Squish Tech, LLC.
#include "xml_attribute.h"
#include "xml_document.h"

namespace libxmljs {

Napi::FunctionReference XmlAttribute::constructor;

XmlAttribute::XmlAttribute(const Napi::CallbackInfo &info) : XmlNode(info) {
  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() != 1 || !info[0].IsExternal()) {
    return;
  }

  xmlNode *attr = info[0].As<Napi::External<xmlNode>>().Data();

  this->xml_obj = attr;
  this->xml_obj->_private = this;
  this->ancestor = NULL;

  if ((xml_obj->doc != NULL) && (xml_obj->doc->_private != NULL)) {
    XmlDocument *doc = static_cast<XmlDocument *>(this->xml_obj->doc->_private);
    this->Value().Set("document", doc->Value());
  }

  this->ref_wrapped_ancestor();
}

Napi::Value XmlAttribute::NewInstance(Napi::Env env, xmlNode *xml_obj,
                                      const xmlChar *name,
                                      const xmlChar *value) {
  Napi::EscapableHandleScope scope(env);

  xmlAttr *attr = xmlSetProp(xml_obj, name, value);
  assert(attr);

  if (attr->_private) {
    return scope.Escape(static_cast<XmlNode *>(xml_obj->_private)->Value());
  }

  auto external = Napi::External<xmlAttr>::New(env, attr);
  Napi::Object obj = constructor.New({external});
  return scope.Escape(obj);
}

Napi::Value XmlAttribute::NewInstance(Napi::Env env, xmlAttr *attr) {
  Napi::EscapableHandleScope scope(env);
  assert(attr->type == XML_ATTRIBUTE_NODE);

  if (attr->_private) {
    return scope.Escape(static_cast<XmlNode *>(attr->_private)->Value());
  }

  auto external = Napi::External<xmlAttr>::New(env, attr);
  Napi::Object obj = constructor.New({external});
  return scope.Escape(obj);
}

Napi::Value XmlAttribute::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return this->get_name(env);
}

Napi::Value XmlAttribute::AttrValue(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  // attr.value('new value');
  if (info.Length() > 0) {
    std::string value_str = info[0].As<Napi::String>().Utf8Value();
    this->set_value(value_str.c_str());
    return info.This();
  }

  // attr.value();
  return scope.Escape(this->get_value(env));
}

Napi::Value XmlAttribute::Node(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_element(env));
}

Napi::Value XmlAttribute::Namespace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_namespace(env));
}

Napi::Value XmlAttribute::get_name(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name) {
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name,
                                          xmlStrlen(xml_obj->name)));
  }

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
  // if (xml_obj->children)
  //   xmlFreeNodeList(xml_obj->children);

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
  return scope.Escape(XmlElement::NewInstance(env, xml_obj->parent));
}

Napi::Value XmlAttribute::get_namespace(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (!xml_obj->ns) {
    return scope.Escape(env.Null());
  }
  return scope.Escape(XmlNamespace::NewInstance(env, xml_obj->ns));
}

Napi::Function XmlAttribute::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function ctor =
      DefineClass(env, "Attribute",
                  {
                      InstanceMethod("name", &XmlAttribute::Name),
                      InstanceMethod("value", &XmlAttribute::AttrValue),
                      InstanceMethod("node", &XmlAttribute::Node),
                      InstanceMethod("namespace", &XmlAttribute::Namespace),

                      InstanceMethod("doc", &XmlNode::Doc),
                      InstanceMethod("parent", &XmlNode::Parent),
                      InstanceMethod("namespace", &XmlNode::Namespace),
                      InstanceMethod("namespaces", &XmlNode::Namespaces),
                      InstanceMethod("prevSibling", &XmlNode::PrevSibling),
                      InstanceMethod("nextSibling", &XmlNode::NextSibling),
                      InstanceMethod("line", &XmlNode::LineNumber),
                      InstanceMethod("type", &XmlNode::Type),
                      InstanceMethod("toString", &XmlNode::ToString),
                      InstanceMethod("remove", &XmlNode::Remove),
                      InstanceMethod("clone", &XmlNode::Clone),
                  });

  constructor = Napi::Persistent(ctor);
  constructor.SuppressDestruct();
  env.AddCleanupHook([]() { constructor.Reset(); });

  exports.Set("Attribute", ctor);

  return ctor;
}

} // namespace libxmljs
