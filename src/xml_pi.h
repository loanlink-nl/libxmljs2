#ifndef SRC_XML_PI_H_
#define SRC_XML_PI_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlProcessingInstruction : public XmlNode {
public:
  explicit XmlProcessingInstruction(xmlNode *node);

  static void Initialize(Napi::Env env, Napi::Object target);

  static Napi::FunctionReference constructor;

  // create new xml comment to wrap the node
  static Napi::Object New(xmlNode *node);

protected:
  Napi::Value New(const Napi::CallbackInfo& info);
  Napi::Value Name(const Napi::CallbackInfo& info);
  Napi::Value Text(const Napi::CallbackInfo& info);

  void set_name(const char *name);

  Napi::Value get_name(Napi::Env env);
  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_PI_H_
