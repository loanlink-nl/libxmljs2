// Copyright 2009, Squish Tech, LLC.
#ifndef SRC_XML_DOCUMENT_H_
#define SRC_XML_DOCUMENT_H_

#include <libxml/tree.h>

#include "libxmljs.h"

namespace libxmljs {

class XmlDocument : public Napi::ObjectWrap<XmlDocument> {

public:
  // used to create new instanced of a document handle
  static Napi::FunctionReference constructor;

  // TODO make private with accessor
  xmlDoc *xml_obj;

  virtual ~XmlDocument();

  // setup the document handle bindings and internal constructor
  static void Initialize(Napi::Object target);

  // create a new document handle initialized with the
  // given xmlDoc object, intended for use in c++ space
  static Napi::Object New(xmlDoc *doc);

  // publicly expose ref functions
  using ::libxmljs::XmlDocument::ObjectWrap::Ref;
  using ::libxmljs::XmlDocument::ObjectWrap::Unref;

  // expose ObjectWrap::refs_ (for testing)
  // int refs() { return refs_; }

protected:
  // initialize a new document
  explicit XmlDocument(xmlDoc *doc);

  static Napi::Value New(const Napi::CallbackInfo &info);
  static Napi::Value FromHtml(const Napi::CallbackInfo &info);
  static Napi::Value FromXml(const Napi::CallbackInfo &info);
  static Napi::Value SetDtd(const Napi::CallbackInfo &info);

  // document handle methods
  static Napi::Value Root(const Napi::CallbackInfo &info);
  static Napi::Value GetDtd(const Napi::CallbackInfo &info);
  static Napi::Value Encoding(const Napi::CallbackInfo &info);
  static Napi::Value Version(const Napi::CallbackInfo &info);
  static Napi::Value Doc(const Napi::CallbackInfo &info);
  static Napi::Value Errors(const Napi::CallbackInfo &info);
  static Napi::Value ToString(const Napi::CallbackInfo &info);
  static Napi::Value Validate(const Napi::CallbackInfo &info);
  static Napi::Value RngValidate(const Napi::CallbackInfo &info);
  static Napi::Value SchematronValidate(const Napi::CallbackInfo &info);
  static Napi::Value type(const Napi::CallbackInfo &info);

  // Static member variables
  static const int DEFAULT_PARSING_OPTS;
  static const int EXCLUDE_IMPLIED_ELEMENTS;

  void setEncoding(const char *encoding);
};

} // namespace libxmljs

#endif // SRC_XML_DOCUMENT_H_
