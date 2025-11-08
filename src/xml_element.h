// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_ELEMENT_H_
#define SRC_XML_ELEMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlElement : public XmlNode {
public:
  explicit XmlElement(xmlNode *node);

  static void Initialize(Napi::Object target);

  static Napi::FunctionReference constructor;

  // create new xml element to wrap the node
  static Napi::Object New(xmlNode *node);

protected:
  static Napi::Value New(const Napi::CallbackInfo& info);
  static Napi::Value Name(const Napi::CallbackInfo& info);
  static Napi::Value Attr(const Napi::CallbackInfo& info);
  static Napi::Value Attrs(const Napi::CallbackInfo& info);
  static Napi::Value Find(const Napi::CallbackInfo& info);
  static Napi::Value Text(const Napi::CallbackInfo& info);
  static Napi::Value Path(const Napi::CallbackInfo& info);
  static Napi::Value Child(const Napi::CallbackInfo& info);
  static Napi::Value ChildNodes(const Napi::CallbackInfo& info);
  static Napi::Value AddChild(const Napi::CallbackInfo& info);
  static Napi::Value AddCData(const Napi::CallbackInfo& info);
  static Napi::Value NextElement(const Napi::CallbackInfo& info);
  static Napi::Value PrevElement(const Napi::CallbackInfo& info);
  static Napi::Value AddPrevSibling(const Napi::CallbackInfo& info);
  static Napi::Value AddNextSibling(const Napi::CallbackInfo& info);
  static Napi::Value Replace(const Napi::CallbackInfo& info);

  void set_name(const char *name);

  Napi::Value get_name();
  Napi::Value get_child(int32_t idx);
  Napi::Value get_child_nodes();
  Napi::Value get_path();
  Napi::Value get_attr(const char *name);
  Napi::Value get_attrs();
  void set_attr(const char *name, const char *value);
  void add_cdata(xmlNode *cdata);
  void unlink_children();
  void set_content(const char *content);
  Napi::Value get_content();
  Napi::Value get_next_element();
  Napi::Value get_prev_element();
  void replace_element(xmlNode *element);
  void replace_text(const char *content);
  bool child_will_merge(xmlNode *child);
};

} // namespace libxmljs

#endif // SRC_XML_ELEMENT_H_
