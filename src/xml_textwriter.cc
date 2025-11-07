// Copyright 2011, Squish Tech, LLC.

#include "xml_textwriter.h"
#include "libxmljs.h"

using namespace Napi;
namespace libxmljs {

#define THROW_ON_ERROR(text)                                                   \
  if (result == -1) {                                                          \
    Napi::Error::New(env, text).ThrowAsJavaScriptException();                  \
    return;                                                                    \
  }

XmlTextWriter::XmlTextWriter() {
  textWriter = NULL;
  writerBuffer = NULL;
}

XmlTextWriter::~XmlTextWriter() {
  if (textWriter) {
    xmlFreeTextWriter(textWriter);
  }
  if (writerBuffer) {
    xmlBufferFree(writerBuffer);
  }
}

Napi::Value XmlTextWriter::NewTextWriter(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlTextWriter *writer = new XmlTextWriter();
  writer->Wrap(info.This());
  writer->OpenMemory(info);

  return return info.This();
}

Napi::Value XmlTextWriter::OpenMemory(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  writer->writerBuffer = xmlBufferCreate();
  if (!writer->writerBuffer) {
    Napi::Error::New(env, "Failed to create memory buffer")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  writer->textWriter = xmlNewTextWriterMemory(writer->writerBuffer, 0);
  if (!writer->textWriter) {
    xmlBufferFree(writer->writerBuffer);
    Napi::Error::New(env, "Failed to create buffer writer")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  return return env.Undefined();
}

Napi::Value XmlTextWriter::BufferContent(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  // Flush the output buffer of the libxml writer instance in order to push all
  // the content to our writerBuffer.
  xmlTextWriterFlush(writer->textWriter);

  // Receive bytes from the writerBuffer
  const xmlChar *buf = xmlBufferContent(writer->writerBuffer);

  return return Napi::String::New(env, (const char *)buf,
                                  xmlBufferLength(writer->writerBuffer));
}

void XmlTextWriter::clearBuffer() {
  // Flush the output buffer of the libxml writer instance in order to push all
  // the content to our writerBuffer.
  xmlTextWriterFlush(textWriter);
  // Clear the memory buffer
  xmlBufferEmpty(writerBuffer);
}

Napi::Value XmlTextWriter::BufferEmpty(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  writer->clearBuffer();

  return return env.Undefined();
}

Napi::Value XmlTextWriter::StartDocument(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  std::string version = info[0].As<Napi::String>();
  std::string encoding = info[1].As<Napi::String>();
  const char *standalone = NULL;

  if (info[2].IsBoolean()) {
    const char *wordBool =
        info[2].As<Napi::Boolean>().Value().FromMaybe(false) ? "yes" : "no";
    standalone = Napi::String >
                 (wordBool->As < Napi::String::New(env).Utf8Value().c_str());
  } else if (info[2].IsString()) {
    standalone = info[2].As<Napi::String>().Utf8Value().c_str();
  }

  int result = xmlTextWriterStartDocument(
      writer->textWriter, info[0].IsUndefined() ? NULL : *version,
      info[1].IsUndefined() ? NULL : *encoding, standalone);

  THROW_ON_ERROR("Failed to start document");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::EndDocument(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterEndDocument(writer->textWriter);

  THROW_ON_ERROR("Failed to end document");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::StartElementNS(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  std::string prefix = info[0].As<Napi::String>();
  std::string name = info[1].As<Napi::String>();
  std::string namespaceURI = info[2].As<Napi::String>();

  int result = xmlTextWriterStartElementNS(
      writer->textWriter,
      info[0].IsUndefined() ? NULL : (const xmlChar *)*prefix,
      info[1].IsUndefined() ? NULL : (const xmlChar *)*name,
      info[2].IsUndefined() ? NULL : (const xmlChar *)*namespaceURI);

  THROW_ON_ERROR("Failed to start element");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::EndElement(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterEndElement(writer->textWriter);

  THROW_ON_ERROR("Failed to end element");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::StartAttributeNS(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  std::string prefix = info[0].As<Napi::String>();
  std::string name = info[1].As<Napi::String>();
  std::string namespaceURI = info[2].As<Napi::String>();

  int result = xmlTextWriterStartAttributeNS(
      writer->textWriter,
      info[0].IsUndefined() ? NULL : (const xmlChar *)*prefix,
      info[1].IsUndefined() ? NULL : (const xmlChar *)*name,
      info[2].IsUndefined() ? NULL : (const xmlChar *)*namespaceURI);

  THROW_ON_ERROR("Failed to start attribute");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::EndAttribute(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterEndAttribute(writer->textWriter);

  THROW_ON_ERROR("Failed to end attribute");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::StartCdata(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterStartCDATA(writer->textWriter);

  THROW_ON_ERROR("Failed to start CDATA section");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::EndCdata(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterEndCDATA(writer->textWriter);

  THROW_ON_ERROR("Failed to end CDATA section");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::StartComment(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterStartComment(writer->textWriter);

  THROW_ON_ERROR("Failed to start Comment section");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::EndComment(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  int result = xmlTextWriterEndComment(writer->textWriter);

  THROW_ON_ERROR("Failed to end Comment section");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::WriteString(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);

  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  std::string string = info[0].As<Napi::String>();

  int result =
      xmlTextWriterWriteString(writer->textWriter, (const xmlChar *)*string);

  THROW_ON_ERROR("Failed to write string");

  return return Napi::Number::New(env, (double)result);
}

Napi::Value XmlTextWriter::OutputMemory(const Napi::CallbackInfo &info) {
  bool clear =
      info.Length() == 0 || info[0].As<Napi::Boolean>().Value().FromMaybe(true);
  XmlTextWriter *writer = info.This().Unwrap<XmlTextWriter>();

  BufferContent(info);

  if (clear) {
    writer->clearBuffer();
  }
}

void XmlTextWriter::Initialize(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);

  Napi::FunctionReference writer_t = Napi::Function::New(env, NewTextWriter);

  Napi::FunctionReference xml_writer_template;
  xml_writer_template.Reset(writer_t);

  Napi::SetPrototypeMethod(writer_t, "toString", XmlTextWriter::BufferContent);

  Napi::SetPrototypeMethod(writer_t, "outputMemory",
                           XmlTextWriter::OutputMemory);

  Napi::SetPrototypeMethod(writer_t, "clear", XmlTextWriter::BufferEmpty);

  Napi::SetPrototypeMethod(writer_t, "startDocument",
                           XmlTextWriter::StartDocument);

  Napi::SetPrototypeMethod(writer_t, "endDocument", XmlTextWriter::EndDocument);

  Napi::SetPrototypeMethod(writer_t, "startElementNS",
                           XmlTextWriter::StartElementNS);

  Napi::SetPrototypeMethod(writer_t, "endElement", XmlTextWriter::EndElement);

  Napi::SetPrototypeMethod(writer_t, "startAttributeNS",
                           XmlTextWriter::StartAttributeNS);

  Napi::SetPrototypeMethod(writer_t, "endAttribute",
                           XmlTextWriter::EndAttribute);

  Napi::SetPrototypeMethod(writer_t, "startCdata", XmlTextWriter::StartCdata);

  Napi::SetPrototypeMethod(writer_t, "endCdata", XmlTextWriter::EndCdata);

  Napi::SetPrototypeMethod(writer_t, "startComment",
                           XmlTextWriter::StartComment);

  Napi::SetPrototypeMethod(writer_t, "endComment", XmlTextWriter::EndComment);

  Napi::SetPrototypeMethod(writer_t, "writeString", XmlTextWriter::WriteString);

  (target).Set(Napi::String::New(env, "TextWriter"),
               Napi::GetFunction(writer_t));
}
} // namespace libxmljs
