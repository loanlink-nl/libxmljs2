// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NODE_H_
#define SRC_XML_NODE_H_

#include "libxmljs.h"
#include <libxml/tree.h>

namespace libxmljs {

class XmlNode : public Napi::ObjectWrap<XmlNode> {
public:
  xmlNode *xml_obj;

  // reference to a parent XmlNode or XmlElement
  xmlNode *ancestor;

  // referencing functions
  void ref_wrapped_ancestor();
  void unref_wrapped_ancestor();
  xmlNode *get_wrapped_ancestor();

  // the doc ref'd by this proxy
  xmlDoc *doc;

  explicit XmlNode(xmlNode *node);
  virtual ~XmlNode();

  static void Initialize(Napi::Env env, Napi::Object target);
  static Napi::FunctionReference constructor;

  // create new XmlElement, XmlAttribute, etc. to wrap a libxml xmlNode
  static Napi::Value New(Napi::Env env, xmlNode *node);
  
  // N-API constructor
  XmlNode(const Napi::CallbackInfo& info);

protected:
  Napi::Value Doc(const Napi::CallbackInfo& info);
  Napi::Value Namespace(const Napi::CallbackInfo& info);
  Napi::Value Namespaces(const Napi::CallbackInfo& info);
  Napi::Value Parent(const Napi::CallbackInfo& info);
  Napi::Value NextSibling(const Napi::CallbackInfo& info);
  Napi::Value PrevSibling(const Napi::CallbackInfo& info);
  Napi::Value LineNumber(const Napi::CallbackInfo& info);
  Napi::Value Type(const Napi::CallbackInfo& info);
  Napi::Value ToString(const Napi::CallbackInfo& info);
  Napi::Value Remove(const Napi::CallbackInfo& info);
  Napi::Value Clone(const Napi::CallbackInfo& info);

  Napi::Value get_doc(Napi::Env env);
  Napi::Value remove_namespace(Napi::Env env);
  Napi::Value get_namespace(Napi::Env env);
  void set_namespace(xmlNs *ns);
  xmlNs *find_namespace(const char *search_str);
  Napi::Value get_all_namespaces(Napi::Env env);
  Napi::Value get_local_namespaces(Napi::Env env);
  Napi::Value get_parent(Napi::Env env);
  Napi::Value get_prev_sibling(Napi::Env env);
  Napi::Value get_next_sibling(Napi::Env env);
  Napi::Value get_line_number(Napi::Env env);
  Napi::Value clone(Napi::Env env, bool recurse);
  Napi::Value get_type(Napi::Env env);
  Napi::Value to_string(Napi::Env env, int options = 0);
  void remove();
  void add_child(xmlNode *child);
  void add_prev_sibling(xmlNode *element);
  void add_next_sibling(xmlNode *element);
  void replace_element(xmlNode *element);
  void replace_text(const char *content);
  xmlNode *import_node(xmlNode *node);
};

} // namespace libxmljs

#endif // SRC_XML_NODE_H_
