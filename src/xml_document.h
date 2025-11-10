// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_DOCUMENT_H_
#define SRC_XML_DOCUMENT_H_

#include <libxml/tree.h>

#include <napi.h>

namespace libxmljs {

class XmlDocument : public Napi::ObjectWrap<XmlDocument> {

public:
  explicit XmlDocument(const Napi::CallbackInfo &info);
  virtual ~XmlDocument();

  // used to create new instances of a document handle
  static Napi::FunctionReference constructor;

  // TODO make private with accessor
  xmlDoc *xml_obj;

  // setup the document handle bindings and internal constructor
  static void Init(Napi::Env env, Napi::Object exports);

  // create a new document handle initialized with the
  // given xmlDoc object, intended for use in c++ space
  static Napi::Value NewInstance(Napi::Env env, xmlDoc *doc);

  // publicly expose ref functions
  // using Napi::ObjectWrap<XmlDocument>::Ref;
  // using Napi::ObjectWrap<XmlDocument>::Unref;

  // expose ObjectWrap::refs_ (for testing)
  int refs() { return Ref(); }

protected:
  static Napi::Value FromHtml(const Napi::CallbackInfo &info);
  static Napi::Value FromXml(const Napi::CallbackInfo &info);

  Napi::Value SetDtd(const Napi::CallbackInfo &info);

  // document handle methods
  Napi::Value Root(const Napi::CallbackInfo &info);
  Napi::Value GetDtd(const Napi::CallbackInfo &info);
  Napi::Value Encoding(const Napi::CallbackInfo &info);
  Napi::Value Version(const Napi::CallbackInfo &info);
  Napi::Value Doc(const Napi::CallbackInfo &info);
  Napi::Value Errors(const Napi::CallbackInfo &info);
  Napi::Value ToString(const Napi::CallbackInfo &info);
  Napi::Value Validate(const Napi::CallbackInfo &info);
  Napi::Value RngValidate(const Napi::CallbackInfo &info);
  Napi::Value SchematronValidate(const Napi::CallbackInfo &info);
  Napi::Value Type(const Napi::CallbackInfo &info);

  // Static member variables
  static const int DEFAULT_PARSING_OPTS;
  static const int EXCLUDE_IMPLIED_ELEMENTS;

  void setEncoding(const char *encoding);
};

} // namespace libxmljs

#endif // SRC_XML_DOCUMENT_H_
