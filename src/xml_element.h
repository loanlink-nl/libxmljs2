// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ELEMENT_H_
#define SRC_XML_ELEMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlElement : public XmlNode<XmlElement> {
public:
  XmlElement(const Napi::CallbackInfo &info);

  static Napi::Function Init(Napi::Env env, Napi::Object exports);

  // create new xml element to wrap the node
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

protected:
  Napi::Value Name(const Napi::CallbackInfo &info);
  Napi::Value Attr(const Napi::CallbackInfo &info);
  Napi::Value Attrs(const Napi::CallbackInfo &info);
  Napi::Value Find(const Napi::CallbackInfo &info);
  Napi::Value Text(const Napi::CallbackInfo &info);
  Napi::Value Path(const Napi::CallbackInfo &info);
  Napi::Value Child(const Napi::CallbackInfo &info);
  Napi::Value ChildNodes(const Napi::CallbackInfo &info);
  Napi::Value AddChild(const Napi::CallbackInfo &info);
  Napi::Value AddCData(const Napi::CallbackInfo &info);
  Napi::Value NextElement(const Napi::CallbackInfo &info);
  Napi::Value PrevElement(const Napi::CallbackInfo &info);
  Napi::Value AddPrevSibling(const Napi::CallbackInfo &info);
  Napi::Value AddNextSibling(const Napi::CallbackInfo &info);
  Napi::Value Replace(const Napi::CallbackInfo &info);

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

private:
  static Napi::FunctionReference constructor;
};

} // namespace libxmljs

#endif // SRC_XML_ELEMENT_H_
