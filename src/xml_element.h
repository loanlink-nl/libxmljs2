// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ELEMENT_H_
#define SRC_XML_ELEMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlElement : public XmlNode<XmlElement> {
public:
  XmlElement(const Napi::CallbackInfo &info);
  virtual ~XmlElement();

  static Napi::Function GetClass(Napi::Env env, Napi::Object exports);

  static Napi::FunctionReference constructor;

  // create new xml element to wrap the node
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

protected:
  static Napi::Value Name(const Napi::CallbackInfo &info);
  static Napi::Value Attr(const Napi::CallbackInfo &info);
  static Napi::Value Attrs(const Napi::CallbackInfo &info);
  static Napi::Value Find(const Napi::CallbackInfo &info);
  static Napi::Value Text(const Napi::CallbackInfo &info);
  static Napi::Value Path(const Napi::CallbackInfo &info);
  static Napi::Value Child(const Napi::CallbackInfo &info);
  static Napi::Value ChildNodes(const Napi::CallbackInfo &info);
  static Napi::Value AddChild(const Napi::CallbackInfo &info);
  static Napi::Value AddCData(const Napi::CallbackInfo &info);
  static Napi::Value NextElement(const Napi::CallbackInfo &info);
  static Napi::Value PrevElement(const Napi::CallbackInfo &info);
  static Napi::Value AddPrevSibling(const Napi::CallbackInfo &info);
  static Napi::Value AddNextSibling(const Napi::CallbackInfo &info);
  static Napi::Value Replace(const Napi::CallbackInfo &info);

  void set_name(const char *name);

  Napi::Value get_name(Napi::Env env);
  Napi::Value get_child(Napi::Env env, int32_t idx);
  Napi::Value get_child_nodes(Napi::Env env);
  Napi::Value get_path(Napi::Env env);
  Napi::Value get_attr(Napi::Env env, const char *name);
  Napi::Value get_attrs(Napi::Env env);
  void set_attr(const char *name, const char *value);
  void add_cdata(xmlNode *cdata);
  void unlink_children();
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
  Napi::Value get_next_element(Napi::Env env);
  Napi::Value get_prev_element(Napi::Env env);
  void replace_element(xmlNode *element);
  void replace_text(const char *content);
  bool child_will_merge(xmlNode *child);
};

} // namespace libxmljs

#endif // SRC_XML_ELEMENT_H_
