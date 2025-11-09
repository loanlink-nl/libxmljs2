// Copyright 2009, Squish Tech, LLC.

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_text.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlText::constructor;

Napi::Value XmlText::get_path(Napi::Env env) {
  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);
  return js_obj;
}

// doc, name, content
XmlText::XmlText(const Napi::CallbackInfo &info) : XmlNode<XmlText>(info) {
  Napi::Env env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Text constructor must be called with new")
        .ThrowAsJavaScriptException();
    return;
  }

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return;
  }

  if (!info[0].IsObject()) {
    Napi::TypeError::New(env, "Document argument must be an object")
        .ThrowAsJavaScriptException();
    return;
  }

  if (!info[1].IsString()) {
    Napi::TypeError::New(env, "content argument must be of type string")
        .ThrowAsJavaScriptException();
    return;
  }

  Napi::Object docObj = info[0].As<Napi::Object>();
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(docObj);
  if (document == nullptr) {
    Napi::Error::New(env, "Invalid document argument")
        .ThrowAsJavaScriptException();
    return;
  }

  std::string content = info[1].As<Napi::String>().Utf8Value();

  xmlNode *textNode =
      xmlNewDocText(document->xml_obj, (const xmlChar *)content.c_str());

  XmlText *element = this;
  textNode->_private = element;

  // this prevents the document from going away
  info.This().As<Napi::Object>().Set("document", info[0]);
}

Napi::Value XmlText::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value();
  }

  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = constructor.New({external});
  return scope.Escape(instance).ToObject();
}

Napi::Value XmlText::NextElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *element =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (element == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  return element->get_next_element(env);
}

Napi::Value XmlText::PrevElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *element =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (element == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  return element->get_prev_element(env);
}

Napi::Value XmlText::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *element =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (element == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info.Length() == 0) {
    return element->get_content(env);
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    element->set_content(content.c_str());
  }

  return info.This();
}

Napi::Value XmlText::AddPrevSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *text =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (text == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object siblingObj = info[0].As<Napi::Object>();
  XmlNode *new_sibling = Napi::ObjectWrap<XmlNode>::Unwrap(siblingObj);
  if (new_sibling == nullptr) {
    Napi::Error::New(env, "Invalid sibling node argument")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  xmlNode *imported_sibling = text->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             text->prev_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  text->add_prev_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::AddNextSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *text =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (text == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Object siblingObj = info[0].As<Napi::Object>();
  XmlNode *new_sibling = Napi::ObjectWrap<XmlNode>::Unwrap(siblingObj);
  if (new_sibling == nullptr) {
    Napi::Error::New(env, "Invalid sibling node argument")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  xmlNode *imported_sibling = text->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             text->next_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  text->add_next_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::Replace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *element =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (element == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info[0].IsString()) {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    element->replace_text(content.c_str());
  } else {
    Napi::Object siblingObj = info[0].As<Napi::Object>();
    XmlText *new_sibling = Napi::ObjectWrap<XmlText>::Unwrap(siblingObj);
    if (new_sibling == nullptr) {
      Napi::Error::New(env, "Invalid replacement node argument")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }

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

Napi::Value XmlText::Path(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *text =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (text == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  return text->get_path(env);
}

Napi::Value XmlText::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  XmlText *text =
      Napi::ObjectWrap<XmlText>::Unwrap(info.This().As<Napi::Object>());

  if (text == nullptr) {
    Napi::Error::New(env, "Invalid XmlText instance")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info.Length() == 0)
    return text->get_name(env);
  return info.This();
}

void XmlText::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlText::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return ret_content;
  }

  return Napi::String::New(env, "");
}

Napi::Value XmlText::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, (const char *)xml_obj->name);
  } else {
    return env.Undefined();
  }
}

Napi::Value XmlText::get_next_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->next;
  if (!sibling) {
    return env.Null();
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return XmlText::NewInstance(env, sibling);
  }

  return env.Null();
}

Napi::Value XmlText::get_prev_element(Napi::Env env) {
  xmlNode *sibling = xml_obj->prev;
  if (!sibling) {
    return env.Null();
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return XmlText::NewInstance(env, sibling);
  }

  return env.Null();
}

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

Napi::Function XmlText::GetClass(Napi::Env env, Napi::Object exports) {
  Napi::Function func =
      DefineClass(env, "Text",
                  {
                      ObjectWrap<XmlText>::StaticMethod("nextElement", &XmlText::NextElement),
                      ObjectWrap<XmlText>::StaticMethod("prevElement", &XmlText::PrevElement),
                      ObjectWrap<XmlText>::StaticMethod("text", &XmlText::Text),
                      ObjectWrap<XmlText>::StaticMethod("replace", &XmlText::Replace),
                      ObjectWrap<XmlText>::StaticMethod("path", &XmlText::Path),
                      ObjectWrap<XmlText>::StaticMethod("name", &XmlText::Name),
                      ObjectWrap<XmlText>::StaticMethod("addPrevSibling", &XmlText::AddPrevSibling),
                      ObjectWrap<XmlText>::StaticMethod("addNextSibling", &XmlText::AddNextSibling),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Text", func);

  return func;
}

} // namespace libxmljs
