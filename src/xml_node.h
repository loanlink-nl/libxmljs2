// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_NODE_H_
#define SRC_XML_NODE_H_

#include <libxml/tree.h>
#include <napi.h>

namespace libxmljs {

template <class T> class XmlNode : public Napi::ObjectWrap<T> {
public:
  XmlNode(const Napi::CallbackInfo &info);
  virtual ~XmlNode();

  xmlNode *xml_obj;

  // reference to a parent XmlNode or XmlElement
  xmlNode *ancestor;

  // referencing functions
  void ref_wrapped_ancestor();
  void unref_wrapped_ancestor();
  xmlNode *get_wrapped_ancestor();

  // the doc ref'd by this proxy
  xmlDoc *doc;

  static Napi::FunctionReference constructor;

  // create new XmlElement, XmlAttribute, etc. to wrap a libxml xmlNode
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

  static Napi::Function Init(Napi::Env env, Napi::Object exports);

  Napi::Value Doc(const Napi::CallbackInfo &info);
  Napi::Value Namespace(const Napi::CallbackInfo &info);
  Napi::Value Namespaces(const Napi::CallbackInfo &info);
  Napi::Value Parent(const Napi::CallbackInfo &info);
  Napi::Value NextSibling(const Napi::CallbackInfo &info);
  Napi::Value PrevSibling(const Napi::CallbackInfo &info);
  Napi::Value LineNumber(const Napi::CallbackInfo &info);
  Napi::Value Type(const Napi::CallbackInfo &info);
  Napi::Value ToString(const Napi::CallbackInfo &info);
  Napi::Value Remove(const Napi::CallbackInfo &info);
  Napi::Value Clone(const Napi::CallbackInfo &info);

protected:
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

class XmlNodeInstance : public XmlNode<XmlNodeInstance> {
public:
  using XmlNode::XmlNode;
  virtual ~XmlNodeInstance();
};

Napi::Value SetupXmlNodeInheritance(Napi::Env env, Napi::Object exports);

} // namespace libxmljs

#endif // SRC_XML_NODE_H_
