// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_HTML_DOCUMENT_H_
#define SRC_HTML_DOCUMENT_H_

#include "libxmljs.h"
#include "xml_document.h"

namespace libxmljs {

class HtmlDocument : public XmlDocument {
public:
  explicit HtmlDocument(const Napi::CallbackInfo &info) : XmlDocument(info) {}
  static void Init(Napi::Env env, Napi::Object exports);
};

} // namespace libxmljs

#endif // SRC_HTML_DOCUMENT_H_
