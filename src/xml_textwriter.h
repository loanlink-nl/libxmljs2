// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_TEXTWRITER_H_
#define SRC_XML_TEXTWRITER_H_

#include "libxmljs.h"
#include <libxml/xmlwriter.h>

namespace libxmljs {

class XmlTextWriter : public Napi::ObjectWrap<XmlTextWriter> {
public:
  XmlTextWriter();
  virtual ~XmlTextWriter();
  
  // N-API constructor
  XmlTextWriter(const Napi::CallbackInfo& info);

  static void Initialize(Napi::Env env, Napi::Object target);

  static Napi::Value NewTextWriter(const Napi::CallbackInfo& info);

  static Napi::Value OpenMemory(const Napi::CallbackInfo& info);

  static Napi::Value BufferContent(const Napi::CallbackInfo& info);

  static Napi::Value BufferEmpty(const Napi::CallbackInfo& info);

  static Napi::Value StartDocument(const Napi::CallbackInfo& info);

  static Napi::Value EndDocument(const Napi::CallbackInfo& info);

  static Napi::Value StartElementNS(const Napi::CallbackInfo& info);

  static Napi::Value EndElement(const Napi::CallbackInfo& info);

  static Napi::Value StartAttributeNS(const Napi::CallbackInfo& info);

  static Napi::Value EndAttribute(const Napi::CallbackInfo& info);

  static Napi::Value StartCdata(const Napi::CallbackInfo& info);

  static Napi::Value EndCdata(const Napi::CallbackInfo& info);

  static Napi::Value StartComment(const Napi::CallbackInfo& info);

  static Napi::Value EndComment(const Napi::CallbackInfo& info);

  static Napi::Value WriteString(const Napi::CallbackInfo& info);

  static Napi::Value OutputMemory(const Napi::CallbackInfo& info);

private:
  xmlTextWriterPtr textWriter;
  xmlBufferPtr writerBuffer;

  void clearBuffer();

  bool is_open();

  bool is_inmemory();
};
} // namespace libxmljs

#endif // SRC_XML_TEXTWRITER_H_
