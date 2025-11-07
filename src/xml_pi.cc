#include <napi.h>
#include <uv.h>

#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_document.h"
#include "xml_pi.h"
#include "xml_xpath_context.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlProcessingInstruction::constructor;

// doc, content
Napi::Value XmlProcessingInstruction::New(const Napi::CallbackInfo &info) {
  NAN_CONSTRUCTOR_CHECK(ProcessingInstruction)
  Napi::HandleScope scope(env);

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  if (info.Length() == 0) {
    return return info.This();
  }

  DOCUMENT_ARG_CHECK
  if (!info[1].IsString()) {
    Napi::Error::New(env, "name argument must be of type string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  XmlDocument *document = doc.Unwrap<XmlDocument>();
  assert(document);

  std::string name = info[1].As<Napi::String>();

  Napi::Value contentOpt;
  if (info[2].IsString()) {
    contentOpt = info[2];
  } else if (!info[2].IsNullOrUndefined()) {
    Napi::Error::New(env, "content argument must be of type string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  std::string contentRaw = contentOpt.As<Napi::String>();
  const char *content = (contentRaw.Length()) ? *contentRaw : NULL;

  xmlNode *pi = xmlNewDocPI(document->xml_obj, (const xmlChar *)*name,
                            (xmlChar *)content);

  XmlProcessingInstruction *processing_instruction =
      new XmlProcessingInstruction(pi);
  pi->_private = processing_instruction;
  processing_instruction->Wrap(info.This());

  // this prevents the document from going away
  (info.This()).Set(Napi::String::New(env, "document"), info[0]).Check();

  return return info.This();
}

Napi::Value XmlProcessingInstruction::Name(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlProcessingInstruction *processing_instruction =
      info.This().Unwrap<XmlProcessingInstruction>();
  assert(processing_instruction);

  if (info.Length() == 0)
    return return processing_instruction->get_name();

  std::string name = info[0].As<Napi::String>(.To<Napi::String>());
  processing_instruction->set_name(*name);
  return return info.This();
}

Napi::Value XmlProcessingInstruction::Text(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlProcessingInstruction *processing_instruction =
      info.This().Unwrap<XmlProcessingInstruction>();
  assert(processing_instruction);

  if (info.Length() == 0) {
    return return processing_instruction->get_content();
  } else {
    processing_instruction->set_content(
        info[0].As<Napi::String>().Utf8Value().c_str());
  }

  return return info.This();
}

void XmlProcessingInstruction::set_name(const char *name) {
  xmlNodeSetName(xml_obj, (const xmlChar *)name);
}

Napi::Value XmlProcessingInstruction::get_name() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->name)
    return scope.Escape(Napi::String::New(env, (const char *)xml_obj->name));
  else
    return scope.Escape(env.Undefined());
}

void XmlProcessingInstruction::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Napi::Value XmlProcessingInstruction::get_content() {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Object XmlProcessingInstruction::New(xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->handle());
  }

  XmlProcessingInstruction *processing_instruction =
      new XmlProcessingInstruction(node);
  Napi::Object obj =
      Napi::NewInstance(Napi::GetFunction(Napi::New(env, constructor)));
  processing_instruction->Wrap(obj);
  return scope.Escape(obj);
}

XmlProcessingInstruction::XmlProcessingInstruction(xmlNode *node)
    : XmlNode(node) {}

void XmlProcessingInstruction::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference t = Napi::Napi::FunctionReference::New(
      env, static_cast<NAN_METHOD((*))>(New));
  t->Inherit(Napi::New(env, XmlNode::constructor));

  constructor.Reset(t);

  Napi::SetPrototypeMethod(t, "name", XmlProcessingInstruction::Name);

  Napi::SetPrototypeMethod(t, "text", XmlProcessingInstruction::Text);

  (target).Set(Napi::String::New(env, "ProcessingInstruction"),
               Napi::GetFunction(t));
}

} // namespace libxmljs
