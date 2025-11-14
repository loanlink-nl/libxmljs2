#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_xpath_context.h"

namespace libxmljs {

Napi::FunctionReference XmlComment::constructor;

// JS-signature: (doc: Document, content?: string)
XmlComment::XmlComment(const Napi::CallbackInfo &info) : XmlNode(info) {
  Napi::Env env = info.Env();

  // if we were created for an existing xml node, then we don't need
  // to create a new node on the document
  xmlNode *comm;
  if (info.Length() == 1 && info[0].IsExternal()) {
    comm = info[0].As<Napi::External<xmlNode>>().Data();
  } else {
    DOCUMENT_ARG_CHECK;

    Napi::Object docObj = info[0].ToObject();
    XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(docObj);
    if (document == nullptr) {
      Napi::Error::New(env, "Invalid document argument")
          .ThrowAsJavaScriptException();
      return;
    }

    const char *content = nullptr;
    std::string contentStr;
    if (info.Length() > 1 && info[1].IsString()) {
      contentStr = info[1].As<Napi::String>().Utf8Value();
      content = contentStr.c_str();
    }

    comm = xmlNewDocComment(document->xml_obj, (xmlChar *)content);
  }

  this->xml_obj = comm;
  this->xml_obj->_private = this;
  this->ancestor = NULL;

  if ((xml_obj->doc != NULL) && (xml_obj->doc->_private != NULL)) {
    this->doc = xml_obj->doc;

    XmlDocument *doc = static_cast<XmlDocument *>(this->doc->_private);
    printf("ref doc comm\n");
    fflush(stdout);
    doc->Ref();
    this->Value().Set("document", doc->Value());
  }

  this->Value().Set("_xmlNode",
                    Napi::External<xmlNode>::New(env, this->xml_obj));
  this->ref_wrapped_ancestor();
}

Napi::Value XmlComment::Text(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  if (info.Length() == 0) {
    return scope.Escape(this->get_content(env));
  } else {
    std::string content = info[0].As<Napi::String>().Utf8Value();
    this->set_content(content.c_str());
  }

  return info.This();
}

void XmlComment::set_content(const char *content) {
  xmlNodeSetContent(xml_obj, (xmlChar *)content);
}

Napi::Value XmlComment::get_content(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  xmlChar *content = xmlNodeGetContent(xml_obj);
  if (content) {
    Napi::String ret_content = Napi::String::New(env, (const char *)content);
    xmlFree(content);
    return scope.Escape(ret_content);
  }

  return scope.Escape(Napi::String::New(env, ""));
}

Napi::Value XmlComment::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);

  if (node->_private) {
    return scope.Escape(static_cast<XmlNode *>(node->_private)->Value());
  }

  Napi::Function cons = constructor.Value();
  auto external = Napi::External<xmlNode>::New(env, node);
  Napi::Object instance = cons.New({external});

  return scope.Escape(instance);
}

Napi::Function XmlComment::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func =
      DefineClass(env, "Comment",
                  {
                      InstanceMethod("text", &XmlComment::Text),

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

  exports.Set("Comment", func);

  return func;
}

} // namespace libxmljs
