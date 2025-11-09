#ifndef SRC_XML_TEXT_H_
#define SRC_XML_TEXT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlText : public XmlNode<XmlText> {
public:
  explicit XmlText(const Napi::CallbackInfo &info);

  static Napi::Function GetClass(Napi::Env env, Napi::Object exports);

  static Napi::FunctionReference constructor;

  // create new xml element to wrap the node
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

protected:
  static Napi::Value Text(const Napi::CallbackInfo &info);
  static Napi::Value Replace(const Napi::CallbackInfo &info);
  static Napi::Value Path(const Napi::CallbackInfo &info);
  static Napi::Value Name(const Napi::CallbackInfo &info);

  static Napi::Value NextElement(const Napi::CallbackInfo &info);
  static Napi::Value PrevElement(const Napi::CallbackInfo &info);
  static Napi::Value AddPrevSibling(const Napi::CallbackInfo &info);
  static Napi::Value AddNextSibling(const Napi::CallbackInfo &info);

  Napi::Value get_next_element(Napi::Env env);
  Napi::Value get_prev_element(Napi::Env env);
  Napi::Value get_content(Napi::Env env);
  Napi::Value get_path(Napi::Env env);
  Napi::Value get_name(Napi::Env env);
  void set_content(const char *content);
  void replace_text(const char *content);
  void replace_element(xmlNode *element);
  void add_prev_sibling(xmlNode *element);
  void add_next_sibling(xmlNode *element);
  bool prev_sibling_will_merge(xmlNode *node);
  bool next_sibling_will_merge(xmlNode *node);
};

} // namespace libxmljs

#endif // SRC_XML_TEXT_H_
