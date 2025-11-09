// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_TEXTWRITER_H_
#define SRC_XML_TEXTWRITER_H_

#include "libxmljs.h"
#include <libxml/xmlwriter.h>

namespace libxmljs {

class XmlTextWriter : public Napi::ObjectWrap<XmlTextWriter> {
public:
  XmlTextWriter(const Napi::CallbackInfo& info);
  ~XmlTextWriter();

  static void Initialize(Napi::Env env, Napi::Object exports);
  static Napi::FunctionReference constructor;

  static Napi::Value NewInstance(Napi::Env env);

private:
  Napi::Value OpenMemory(const Napi::CallbackInfo& info);
  Napi::Value BufferContent(const Napi::CallbackInfo& info);
  Napi::Value BufferEmpty(const Napi::CallbackInfo& info);
  Napi::Value StartDocument(const Napi::CallbackInfo& info);
  Napi::Value EndDocument(const Napi::CallbackInfo& info);
  Napi::Value StartElementNS(const Napi::CallbackInfo& info);
  Napi::Value EndElement(const Napi::CallbackInfo& info);
  Napi::Value StartAttributeNS(const Napi::CallbackInfo& info);
  Napi::Value EndAttribute(const Napi::CallbackInfo& info);
  Napi::Value StartCdata(const Napi::CallbackInfo& info);
  Napi::Value EndCdata(const Napi::CallbackInfo& info);
  Napi::Value StartComment(const Napi::CallbackInfo& info);
  Napi::Value EndComment(const Napi::CallbackInfo& info);
  Napi::Value WriteString(const Napi::CallbackInfo& info);
  Napi::Value OutputMemory(const Napi::CallbackInfo& info);

  xmlTextWriterPtr textWriter;
  xmlBufferPtr writerBuffer;

  void clearBuffer();
  bool is_open();
  bool is_inmemory();
};

} // namespace libxmljs

#endif // SRC_XML_TEXTWRITER_H_
