#ifndef SRC_XML_PI_H_
#define SRC_XML_PI_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlProcessingInstruction : public XmlNode<XmlProcessingInstruction> {
public:
  explicit XmlProcessingInstruction(const Napi::CallbackInfo &info);

  static Napi::Function Init(Napi::Env env, Napi::Object exports);

  static Napi::FunctionReference constructor;

  // create new xml processing instruction to wrap the node
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

protected:
  Napi::Value Name(const Napi::CallbackInfo &info);
  Napi::Value Text(const Napi::CallbackInfo &info);

  void set_name(const char *name);

  Napi::Value get_name(Napi::Env env);
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_PI_H_
