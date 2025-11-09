#ifndef SRC_XML_COMMENT_H_
#define SRC_XML_COMMENT_H_

#include "libxmljs.h"
#include "xml_node.h"

namespace libxmljs {

class XmlComment : public XmlNode<XmlComment> {
public:
  explicit XmlComment(const Napi::CallbackInfo &info);

  static Napi::Function GetClass(Napi::Env env, Napi::Object exports);

  static Napi::FunctionReference constructor;

  // create new xml comment to wrap the node
  static Napi::Value NewInstance(Napi::Env env, xmlNode *node);

protected:
  static Napi::Value Text(const Napi::CallbackInfo &info);

  void set_content(const char *content);
  Napi::Value get_content(Napi::Env env);
};

} // namespace libxmljs

#endif // SRC_XML_COMMENT_H_
