// Copyright 2011, Squish Tech, LLC.

#include "xml_textwriter.h"
#include "libxmljs.h"

namespace libxmljs {

#define THROW_ON_ERROR(env, text)                                              \
  if (result == -1) {                                                          \
    Napi::Error::New(env, text).ThrowAsJavaScriptException();                  \
    return env.Undefined();                                                    \
  }

Napi::FunctionReference XmlTextWriter::constructor;

XmlTextWriter::XmlTextWriter(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlTextWriter>(info) {
  textWriter = NULL;
  writerBuffer = NULL;

  // Initialize the writer
  writerBuffer = xmlBufferCreate();
  if (!writerBuffer) {
    Napi::Error::New(info.Env(), "Failed to create memory buffer")
        .ThrowAsJavaScriptException();
    return;
  }

  textWriter = xmlNewTextWriterMemory(writerBuffer, 0);
  if (!textWriter) {
    xmlBufferFree(writerBuffer);
    Napi::Error::New(info.Env(), "Failed to create buffer writer")
        .ThrowAsJavaScriptException();
    return;
  }
}

XmlTextWriter::~XmlTextWriter() {
  if (textWriter) {
    xmlFreeTextWriter(textWriter);
  }
  if (writerBuffer) {
    xmlBufferFree(writerBuffer);
  }
}

Napi::Value XmlTextWriter::OpenMemory(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(env.Undefined());
}

Napi::Value XmlTextWriter::BufferContent(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  // Flush the output buffer of the libxml writer instance in order to push all
  // the content to our writerBuffer.
  xmlTextWriterFlush(textWriter);

  // Receive bytes from the writerBuffer
  const xmlChar *buf = xmlBufferContent(writerBuffer);
  size_t length = xmlBufferLength(writerBuffer);

  return scope.Escape(Napi::String::New(env, (const char *)buf, length));
}

void XmlTextWriter::clearBuffer() {
  // Flush the output buffer of the libxml writer instance in order to push all
  // the content to our writerBuffer.
  xmlTextWriterFlush(textWriter);
  // Clear the memory buffer
  xmlBufferEmpty(writerBuffer);
}

Napi::Value XmlTextWriter::BufferEmpty(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  clearBuffer();
  return scope.Escape(env.Undefined());
}

Napi::Value XmlTextWriter::StartDocument(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  const char *version = nullptr;
  const char *encoding = nullptr;
  const char *standalone = nullptr;
  std::string versionStr, encodingStr, standaloneStr;

  if (info.Length() > 0 && info[0].IsString()) {
    versionStr = info[0].As<Napi::String>().Utf8Value();
    version = versionStr.c_str();
  }

  if (info.Length() > 1 && info[1].IsString()) {
    encodingStr = info[1].As<Napi::String>().Utf8Value();
    encoding = encodingStr.c_str();
  }

  if (info.Length() > 2) {
    if (info[2].IsBoolean()) {
      bool value = info[2].As<Napi::Boolean>().Value();
      standalone = value ? "yes" : "no";
    } else if (info[2].IsString()) {
      standaloneStr = info[2].As<Napi::String>().Utf8Value();
      standalone = standaloneStr.c_str();
    }
  }

  int result =
      xmlTextWriterStartDocument(textWriter, version, encoding, standalone);

  THROW_ON_ERROR(env, "Failed to start document");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::EndDocument(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterEndDocument(textWriter);

  THROW_ON_ERROR(env, "Failed to end document");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::StartElementNS(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  const xmlChar *prefix = nullptr;
  const xmlChar *name = nullptr;
  const xmlChar *namespaceURI = nullptr;
  std::string prefixStr, nameStr, nsStr;

  if (info.Length() > 0 && info[0].IsString()) {
    prefixStr = info[0].As<Napi::String>().Utf8Value();
    prefix = (const xmlChar *)prefixStr.c_str();
  }

  if (info.Length() > 1 && info[1].IsString()) {
    nameStr = info[1].As<Napi::String>().Utf8Value();
    name = (const xmlChar *)nameStr.c_str();
  }

  if (info.Length() > 2 && info[2].IsString()) {
    nsStr = info[2].As<Napi::String>().Utf8Value();
    namespaceURI = (const xmlChar *)nsStr.c_str();
  }

  int result =
      xmlTextWriterStartElementNS(textWriter, prefix, name, namespaceURI);

  THROW_ON_ERROR(env, "Failed to start element");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::EndElement(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterEndElement(textWriter);

  THROW_ON_ERROR(env, "Failed to end element");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::StartAttributeNS(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  const xmlChar *prefix = nullptr;
  const xmlChar *name = nullptr;
  const xmlChar *namespaceURI = nullptr;
  std::string prefixStr, nameStr, nsStr;

  if (info.Length() > 0 && info[0].IsString()) {
    prefixStr = info[0].As<Napi::String>().Utf8Value();
    prefix = (const xmlChar *)prefixStr.c_str();
  }

  if (info.Length() > 1 && info[1].IsString()) {
    nameStr = info[1].As<Napi::String>().Utf8Value();
    name = (const xmlChar *)nameStr.c_str();
  }

  if (info.Length() > 2 && info[2].IsString()) {
    nsStr = info[2].As<Napi::String>().Utf8Value();
    namespaceURI = (const xmlChar *)nsStr.c_str();
  }

  int result =
      xmlTextWriterStartAttributeNS(textWriter, prefix, name, namespaceURI);

  THROW_ON_ERROR(env, "Failed to start attribute");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::EndAttribute(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterEndAttribute(textWriter);

  THROW_ON_ERROR(env, "Failed to end attribute");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::StartCdata(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterStartCDATA(textWriter);

  THROW_ON_ERROR(env, "Failed to start CDATA section");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::EndCdata(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterEndCDATA(textWriter);

  THROW_ON_ERROR(env, "Failed to end CDATA section");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::StartComment(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterStartComment(textWriter);

  THROW_ON_ERROR(env, "Failed to start Comment section");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::EndComment(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int result = xmlTextWriterEndComment(textWriter);

  THROW_ON_ERROR(env, "Failed to end Comment section");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::WriteString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  std::string stringStr;
  if (info.Length() > 0) {
    stringStr = info[0].As<Napi::String>().Utf8Value();
  }

  int result =
      xmlTextWriterWriteString(textWriter, (const xmlChar *)stringStr.c_str());

  THROW_ON_ERROR(env, "Failed to write string");

  return scope.Escape(Napi::Number::New(env, (double)result));
}

Napi::Value XmlTextWriter::OutputMemory(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  bool clear = true;
  if (info.Length() > 0) {
    clear = info[0].ToBoolean().Value();
  }

  Napi::Value content = BufferContent(info);

  if (clear) {
    clearBuffer();
  }

  return scope.Escape(content);
}

void XmlTextWriter::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "TextWriter",
      {
          InstanceMethod("toString", &XmlTextWriter::BufferContent),
          InstanceMethod("outputMemory", &XmlTextWriter::OutputMemory),
          InstanceMethod("clear", &XmlTextWriter::BufferEmpty),
          InstanceMethod("startDocument", &XmlTextWriter::StartDocument),
          InstanceMethod("endDocument", &XmlTextWriter::EndDocument),
          InstanceMethod("startElementNS", &XmlTextWriter::StartElementNS),
          InstanceMethod("endElement", &XmlTextWriter::EndElement),
          InstanceMethod("startAttributeNS", &XmlTextWriter::StartAttributeNS),
          InstanceMethod("endAttribute", &XmlTextWriter::EndAttribute),
          InstanceMethod("startCdata", &XmlTextWriter::StartCdata),
          InstanceMethod("endCdata", &XmlTextWriter::EndCdata),
          InstanceMethod("startComment", &XmlTextWriter::StartComment),
          InstanceMethod("endComment", &XmlTextWriter::EndComment),
          InstanceMethod("writeString", &XmlTextWriter::WriteString),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  env.AddCleanupHook([]() { constructor.Reset(); });

  exports.Set("TextWriter", func);
}

Napi::Value XmlTextWriter::NewInstance(Napi::Env env) {
  Napi::Function cons = constructor.Value();
  return cons.New({});
}

} // namespace libxmljs
