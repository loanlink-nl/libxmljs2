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
  Napi::EscapableHandleScope scope(env);

  xmlChar *path = xmlGetNodePath(xml_obj);
  const char *return_path = path ? reinterpret_cast<char *>(path) : "";
  int str_len = xmlStrlen((const xmlChar *)return_path);
  Napi::String js_obj = Napi::String::New(env, return_path, str_len);
  xmlFree(path);

  return scope.Escape(js_obj);
}

// JS-signature: (doc: Document, content: string)
XmlText::XmlText(const Napi::CallbackInfo &info) : XmlNode<XmlText>(info) {
  Napi::Env env = info.Env();

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  xmlNode *textNode;
  if (info.Length() == 1 && info[0].IsExternal()) {
    textNode = info[0].As<Napi::External<xmlNode>>().Data();
  } else {
    DOCUMENT_ARG_CHECK;

    if (!info[1].IsString()) {
      Napi::TypeError::New(env, "content argument must be of type string")
          .ThrowAsJavaScriptException();
      return;
    }

    Napi::Object docObj = info[0].ToObject();
    XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(docObj);
    if (document == nullptr) {
      Napi::Error::New(env, "Invalid document argument")
          .ThrowAsJavaScriptException();
      return;
    }

    std::string content = info[1].ToString().Utf8Value();

    textNode =
        xmlNewDocText(document->xml_obj, (const xmlChar *)content.c_str());
  }

  this->xml_obj = textNode;
  this->xml_obj->_private = this;
  this->ancestor = NULL;

  if ((this->xml_obj->doc != NULL) && (this->xml_obj->doc->_private != NULL)) {
    XmlDocument *doc = static_cast<XmlDocument *>(this->xml_obj->doc->_private);
    doc->Ref();
    this->Value().Set("document", doc->Value());
  }

  this->Value().Set("_xmlNode",
                    Napi::External<xmlNode>::New(env, this->xml_obj));
  this->ref_wrapped_ancestor();
}

Napi::Value XmlText::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->Value());
  }

  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = constructor.New({external});
  return scope.Escape(instance);
}

Napi::Value XmlText::NextElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_next_element(env));
}

Napi::Value XmlText::PrevElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_prev_element(env));
}

Napi::Value XmlText::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  if (info.Length() == 0) {
    return scope.Escape(this->get_content(env));
  } else {
    std::string content = info[0].ToString().Utf8Value();
    this->set_content(content.c_str());
  }

  return info.This();
}

Napi::Value XmlText::AddPrevSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  Napi::Object siblingObj = info[0].ToObject();
  XmlNode *new_sibling = Napi::ObjectWrap<XmlNode>::Unwrap(siblingObj);
  if (new_sibling == nullptr) {
    Napi::Error::New(env, "Invalid sibling node argument")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             this->prev_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  this->add_prev_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::AddNextSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  Napi::Object siblingObj = info[0].ToObject();
  XmlNode *new_sibling = Napi::ObjectWrap<XmlNode>::Unwrap(siblingObj);
  if (new_sibling == nullptr) {
    Napi::Error::New(env, "Invalid sibling node argument")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
  if (imported_sibling == NULL) {
    Napi::Error::New(
        env, "Could not add sibling. Failed to copy node to new Document.")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  } else if ((new_sibling->xml_obj == imported_sibling) &&
             this->next_sibling_will_merge(imported_sibling)) {
    imported_sibling = xmlCopyNode(imported_sibling, 0);
  }
  this->add_next_sibling(imported_sibling);

  return info[0];
}

Napi::Value XmlText::Replace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  if (info[0].IsString()) {
    std::string content = info[0].ToString().Utf8Value();
    this->replace_text(content.c_str());
  } else {
    Napi::Object siblingObj = info[0].ToObject();
    XmlText *new_sibling = Napi::ObjectWrap<XmlText>::Unwrap(siblingObj);
    if (new_sibling == nullptr) {
      Napi::Error::New(env, "Invalid replacement node argument")
          .ThrowAsJavaScriptException();
      return scope.Escape(env.Undefined());
    }

    xmlNode *imported_sibling = this->import_node(new_sibling->xml_obj);
    if (imported_sibling == NULL) {
      Napi::Error::New(
          env, "Could not replace. Failed to copy node to new Document.")
          .ThrowAsJavaScriptException();
      return scope.Escape(env.Undefined());
    }
    this->replace_element(imported_sibling);
  }

  return info[0];
}

Napi::Value XmlText::Path(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_path(env));
}

Napi::Value XmlText::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0)
    return scope.Escape(this->get_name(env));
  return info.This();
}

void XmlText::set_content(const char *content) {
  xmlChar *encoded =
      xmlEncodeSpecialChars(xml_obj->doc, (const xmlChar *)content);
  xmlNodeSetContent(xml_obj, encoded);
  xmlFree(encoded);
}

Napi::Value XmlText::get_content(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Value XmlText::get_name(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name) {
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name));
  } else {
    return scope.Escape(env.Undefined());
  }
}

Napi::Value XmlText::get_next_element(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  xmlNode *sibling = xml_obj->next;
  if (!sibling) {
    return scope.Escape(env.Null());
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE)
    sibling = sibling->next;

  if (sibling) {
    return scope.Escape(XmlText::NewInstance(env, sibling));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlText::get_prev_element(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  xmlNode *sibling = xml_obj->prev;
  if (!sibling) {
    return scope.Escape(env.Null());
  }

  while (sibling && sibling->type != XML_ELEMENT_NODE) {
    sibling = sibling->prev;
  }

  if (sibling) {
    return scope.Escape(XmlText::NewInstance(env, sibling));
  }

  return scope.Escape(env.Null());
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

Napi::Function XmlText::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "Text",
      {
          InstanceMethod("nextElement", &XmlText::NextElement),
          InstanceMethod("prevElement", &XmlText::PrevElement),
          InstanceMethod("text", &XmlText::Text),
          InstanceMethod("replace", &XmlText::Replace),
          InstanceMethod("path", &XmlText::Path),
          InstanceMethod("name", &XmlText::Name),
          InstanceMethod("addPrevSibling", &XmlText::AddPrevSibling),
          InstanceMethod("addNextSibling", &XmlText::AddNextSibling),

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

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Text", func);

  return func;
}

} // namespace libxmljs
