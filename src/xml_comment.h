#ifndef SRC_XML_COMMENT_H_
#define SRC_XML_COMMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlComment : public XmlNode {
public:
  explicit XmlComment(xmlNode *node);
  
  // N-API constructor
  XmlComment(const Napi::CallbackInfo& info);

  static void Initialize(Napi::Env env, Napi::Object target);

  static Napi::FunctionReference constructor;

  // create new xml comment to wrap the node
  static Napi::Object New(Napi::Env env, xmlNode *node);

protected:
  static Napi::Value New(const Napi::CallbackInfo& info);
  Napi::Value Text(const Napi::CallbackInfo& info);

  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_COMMENT_H_
