#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_pi.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlProcessingInstruction::constructor;

XmlProcessingInstruction::XmlProcessingInstruction(
    const Napi::CallbackInfo &info)
    : XmlNode(info) {
  Napi::Env env = info.Env();

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0 || info[0].IsExternal()) {
    return;
  }

  DOCUMENT_ARG_CHECK;

  if (!info[1].IsString()) {
    Napi::TypeError::New(env, "name argument must be of type string")
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

  std::string name = info[1].As<Napi::String>().Utf8Value();

  const char *content = nullptr;
  std::string contentStr;
  if (info.Length() > 2) {
    if (info[2].IsString()) {
      contentStr = info[2].As<Napi::String>().Utf8Value();
      content = contentStr.c_str();
    } else if (!info[2].IsNull() && !info[2].IsUndefined()) {
      Napi::TypeError::New(env, "content argument must be of type string")
          .ThrowAsJavaScriptException();
      return;
    }
  }

  xmlNode *pi = xmlNewDocPI(document->xml_obj, (const xmlChar *)name.c_str(),
                            (xmlChar *)content);

  this->xml_obj = pi;
  this->xml_obj->_private = this;
  this->ancestor = NULL;

  if ((xml_obj->doc != NULL) && (xml_obj->doc->_private != NULL)) {
    this->doc = xml_obj->doc;

    XmlDocument *doc = static_cast<XmlDocument *>(this->doc->_private);
    doc->Ref();
  }

  this->Value().Set("_xmlNode",
                    Napi::External<xmlNode>::New(env, this->xml_obj));
  this->ref_wrapped_ancestor();

  this->Value().Set("document", info[0]);
}

Napi::Value XmlProcessingInstruction::NewInstance(Napi::Env env,
                                                  xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return static_cast<XmlNode *>(node->_private)->Value();
  }

  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = constructor.New({external});
  return scope.Escape(instance).ToObject();
}

Napi::Value XmlProcessingInstruction::Name(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() == 0)
    return this->get_name(env);

  std::string name = info[0].As<Napi::String>().Utf8Value();
  this->set_name(name.c_str());

  return info.This();
}

Napi::Value XmlProcessingInstruction::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (info.Length() == 0) {
    return this->get_content(env);
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    this->set_content(content.c_str());
  }

  return info.This();
}

void XmlProcessingInstruction::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlProcessingInstruction::get_name(Napi::Env env) {
  if (xml_obj->name) {
    return Napi::String::New(env, (const char *)xml_obj->name);
  } else {
    return env.Undefined();
  }
}

void XmlProcessingInstruction::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Napi::Value XmlProcessingInstruction::get_content(Napi::Env env) {
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return ret_content;
  }

  return Napi::String::New(env, "");
}

Napi::Function XmlProcessingInstruction::Init(Napi::Env env,
                                              Napi::Object exports) {
  Napi::Function func =
      DefineClass(env, "ProcessingInstruction",
                  {
                      InstanceMethod("name", &XmlProcessingInstruction::Name),
                      InstanceMethod("text", &XmlProcessingInstruction::Text),

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

  exports.Set("ProcessingInstruction", func);

  return func;
}

} // namespace libxmljs
