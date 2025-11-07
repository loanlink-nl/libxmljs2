// Copyright 2009, Squish Tech, LLC.

#include <napi.h>
#include <node_buffer.h>
#include <uv.h>

#include <cstring>

// #include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/relaxng.h>
#include <libxml/schematron.h>
#include <libxml/xinclude.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlschemas.h>

#include "xml_document.h"
#include <cassert>
#include "xml_element.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_syntax_error.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlDocument::constructor;

Napi::Value XmlDocument::Encoding(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  // if no args, get the encoding
  if (info.Length() == 0 || info[0].IsUndefined()) {
    if (document->xml_obj->encoding)
      return Napi::String::New(
          env, (const char *)document->xml_obj->encoding,
          xmlStrlen((const xmlChar *)document->xml_obj->encoding));

    return env.Null();
  }

  // set the encoding otherwise
  std::string encoding = info[0].As<Napi::String>().Utf8Value();
  document->setEncoding(encoding.c_str());
  return info.This();
}

void XmlDocument::setEncoding(const char *encoding) {
  if (xml_obj->encoding != NULL) {
    xmlFree((xmlChar *)xml_obj->encoding);
  }

  xml_obj->encoding = xmlStrdup((const xmlChar *)encoding);
}

Napi::Value XmlDocument::Version(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  if (document->xml_obj->version)
    return Napi::String::New(
        env, (const char *)document->xml_obj->version,
        xmlStrlen((const xmlChar *)document->xml_obj->version));

  return env.Null();
}

Napi::Value XmlDocument::Root(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  xmlNode *root = xmlDocGetRootElement(document->xml_obj);

  if (info.Length() == 0 || info[0].IsUndefined()) {
    if (!root) {
      return env.Null();
    }
    return XmlElement::New(env, root);
  }

  if (root != NULL) {
    Napi::Error::New(env, "Holder document already has a root node")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // set the element as the root element for the document
  // allows for proper retrieval of root later
  XmlElement *element =
      Napi::ObjectWrap<XmlElement>::Unwrap(info[0].As<Napi::Object>());
  assert(element);
  xmlDocSetRootElement(document->xml_obj, element->xml_obj);
  element->ref_wrapped_ancestor();
  return info[0];
}

Napi::Value XmlDocument::GetDtd(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  xmlDtdPtr dtd = xmlGetIntSubset(document->xml_obj);

  if (!dtd) {
    return env.Null();
  }

  const char *name = (const char *)dtd->name;
  const char *extId = (const char *)dtd->ExternalID;
  const char *sysId = (const char *)dtd->SystemID;

  Napi::Object dtdObj = Napi::Object::New(env);
  Napi::Value nameValue = env.Null();
  Napi::Value extValue = env.Null();
  Napi::Value sysValue = env.Null();

  if (name != NULL) {
    nameValue = Napi::String::New(env, name, strlen(name));
  }

  if (extId != NULL) {
    extValue = Napi::String::New(env, extId, strlen(extId));
  }

  if (sysId != NULL) {
    sysValue = Napi::String::New(env, sysId, strlen(sysId));
  }

  dtdObj.As<Napi::Object>().Set(Napi::String::New(env, "name"), nameValue);

  dtdObj.As<Napi::Object>().Set(Napi::String::New(env, "externalId"), extValue);

  dtdObj.As<Napi::Object>().Set(Napi::String::New(env, "systemId"), sysValue);

  return dtdObj;
}

Napi::Value XmlDocument::SetDtd(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  std::string name = info[0].As<Napi::String>();

  Napi::Value extIdOpt;
  Napi::Value sysIdOpt;
  if (info.Length() > 1 && info[1].IsString()) {
    extIdOpt = info[1];
  }
  if (info.Length() > 2 && info[2].IsString()) {
    sysIdOpt = info[2];
  }

  std::string extIdRaw = extIdOpt.As<Napi::String>().Utf8Value();
  std::string sysIdRaw = sysIdOpt.As<Napi::String>().Utf8Value();

  // must be set to null in order for xmlCreateIntSubset to ignore them
  const char *extId = (extIdRaw.length()) ? extIdRaw.c_str() : NULL;
  const char *sysId = (sysIdRaw.length()) ? sysIdRaw.c_str() : NULL;

  // No good way of unsetting the doctype if it is previously set...this allows
  // us to.
  xmlDtdPtr dtd = xmlGetIntSubset(document->xml_obj);

  xmlUnlinkNode((xmlNodePtr)dtd);
  xmlFreeNode((xmlNodePtr)dtd);

  xmlCreateIntSubset(document->xml_obj, (const xmlChar *)name.c_str(),
                     (const xmlChar *)extId, (const xmlChar *)sysId);

  return info.This();
}

Napi::Value XmlDocument::ToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  assert(document);

  int options = 0;
  const char *encoding = "UTF-8";
  Napi::Value encodingOpt = env.Null();

  if (info[0].IsObject()) {
    Napi::Object obj = info[0].As<Napi::Object>();

    // drop the xml declaration
    if ((obj)
            .Get(Napi::String::New(env, "declaration"))

            .ToBoolean() == false) {
      options |= XML_SAVE_NO_DECL;
    }

    // format save output
    if ((obj)
            .Get(Napi::String::New(env, "format"))

            .ToBoolean() == true) {
      options |= XML_SAVE_FORMAT;
    }

    // no empty tags (only works with XML) ex: <title></title> becomes <title/>
    if ((obj)
            .Get(Napi::String::New(env, "selfCloseEmpty"))

            .ToBoolean() == false) {
      options |= XML_SAVE_NO_EMPTY;
    }

    // format with non-significant whitespace
    if ((obj)
            .Get(Napi::String::New(env, "whitespace"))

            .ToBoolean() == true) {
      options |= XML_SAVE_WSNONSIG;
    }

    encodingOpt = (obj).Get(Napi::String::New(env, "encoding"));

    Napi::Value type = (obj).Get(Napi::String::New(env, "type"));
    if (type.StrictEquals(Napi::String::New(env, "XML")) ||
        type.StrictEquals(Napi::String::New(env, "xml"))) {
      options |= XML_SAVE_AS_XML; // force XML serialization on HTML doc
    } else if (type.StrictEquals(Napi::String::New(env, "HTML")) ||
               type.StrictEquals(Napi::String::New(env, "html"))) {
      options |= XML_SAVE_AS_HTML; // force HTML serialization on XML doc
      // if the document is XML and we want formatted HTML output
      // we must use the XHTML serializer because the default HTML
      // serializer only formats node->type = HTML_NODE and not XML_NODEs
      if ((options & XML_SAVE_FORMAT) && (options & XML_SAVE_XHTML) == false) {
        options |= XML_SAVE_XHTML;
      }
    } else if (type.StrictEquals(Napi::String::New(env, "XHTML")) ||
               type.StrictEquals(Napi::String::New(env, "xhtml"))) {
      options |= XML_SAVE_XHTML; // force XHTML serialization
    }
  } else if (info.Length() == 0 ||
             info[0].ToBoolean().Value()) {
    options |= XML_SAVE_FORMAT;
  }

  if (encodingOpt.IsString()) {
    std::string encoding_ = encodingOpt.As<Napi::String>().Utf8Value();
    encoding = encoding_.c_str();
  }

  xmlBuffer *buf = xmlBufferCreate();
  xmlSaveCtxt *savectx = xmlSaveToBuffer(buf, encoding, options);
  xmlSaveTree(savectx, (xmlNode *)document->xml_obj);
  xmlSaveFlush(savectx);
  xmlSaveClose(savectx);
  Napi::Value ret = env.Null();
  if (xmlBufferLength(buf) > 0)
    ret = Napi::String::New(env, (char *)xmlBufferContent(buf),
                            xmlBufferLength(buf));
  xmlBufferFree(buf);

  return ret;
}

Napi::Value XmlDocument::type(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "document");
}

// not called from node
// private api
Napi::Object XmlDocument::New(Napi::Env env, xmlDoc *doc) {
  Napi::EscapableHandleScope scope(env);

  if (doc->_private) {
    return scope.Escape(static_cast<XmlDocument *>(doc->_private)->Value().ToObject()).ToObject();
  }

  Napi::Object obj = constructor.New({});

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(obj);

  // replace the document we created
  document->xml_obj->_private = NULL;
  xmlFreeDoc(document->xml_obj);
  document->xml_obj = doc;

  // store ourselves in the document
  // this is how we can get instances or already existing v8 objects
  doc->_private = document;

  return scope.Escape(obj).ToObject();
}

int getParserOption(Napi::Env env, Napi::Object props, const char *key, int value,
                    bool defaultValue = true) {
  Napi::HandleScope scope(env);
  Napi::Value prop = (props).Get(Napi::String::New(env, key));
  return (!prop.IsUndefined() && prop.As<Napi::Boolean>().Value() == defaultValue)
             ? value
             : 0;
}

xmlParserOption getParserOptions(Napi::Env env, Napi::Object props) {
  int ret = 0;

  // http://xmlsoft.org/html/libxml-parser.html#xmlParserOption
  // http://www.xmlsoft.org/html/libxml-HTMLparser.html#htmlParserOption

  ret |= getParserOption(env, props, "recover",
                         XML_PARSE_RECOVER); // 1: recover on errors
  /*ret |= getParserOption(env, props, "recover", HTML_PARSE_RECOVER);           //
   * 1: Relaxed parsing*/

  ret |= getParserOption(env, props, "noent",
                         XML_PARSE_NOENT); // 2: substitute entities

  ret |= getParserOption(env, props, "dtdload",
                         XML_PARSE_DTDLOAD); // 4: load the external subset
  ret |= getParserOption(env, props, "doctype", HTML_PARSE_NODEFDTD,
                         false); // 4: do not default a doctype if not found

  ret |= getParserOption(env, props, "dtdattr",
                         XML_PARSE_DTDATTR); // 8: default DTD attributes
  ret |= getParserOption(env, props, "dtdvalid",
                         XML_PARSE_DTDVALID); // 16: validate with the DTD

  ret |= getParserOption(env, props, "noerror",
                         XML_PARSE_NOERROR); // 32: suppress error reports
  ret |= getParserOption(env, props, "errors", HTML_PARSE_NOERROR,
                         false); // 32: suppress error reports

  ret |= getParserOption(env, props, "nowarning",
                         XML_PARSE_NOWARNING); // 64: suppress warning reports
  ret |= getParserOption(env, props, "warnings", HTML_PARSE_NOWARNING,
                         false); // 64: suppress warning reports

  ret |= getParserOption(env, props, "pedantic",
                         XML_PARSE_PEDANTIC); // 128: pedantic error reporting
  /*ret |= getParserOption(env, props, "pedantic", HTML_PARSE_PEDANTIC);         //
   * 128: pedantic error reporting*/

  ret |= getParserOption(env, props, "noblanks",
                         XML_PARSE_NOBLANKS); // 256: remove blank nodes
  ret |= getParserOption(env, props, "blanks", HTML_PARSE_NOBLANKS,
                         false); // 256: remove blank nodes

  ret |= getParserOption(env, props, "sax1", XML_PARSE_SAX1); // 512: use the SAX1 interface internally
  ret |= getParserOption(env, props, "xinclude",
      XML_PARSE_XINCLUDE); // 1024: Implement XInclude substitition

  ret |= getParserOption(env, props, "nonet",
                         XML_PARSE_NONET); // 2048: Forbid network access
  ret |= getParserOption(env, props, "net", HTML_PARSE_NONET,
                         false); // 2048: Forbid network access

  ret |= getParserOption(env, props, "nodict",
      XML_PARSE_NODICT); // 4096: Do not reuse the context dictionnary
  ret |= getParserOption(env, props, "dict", XML_PARSE_NODICT,
                         false); // 4096: Do not reuse the context dictionnary

  ret |= getParserOption(env, props, "nsclean",
      XML_PARSE_NSCLEAN); // 8192: remove redundant namespaces declarations
  ret |= getParserOption(env, props, "implied", HTML_PARSE_NOIMPLIED,
                         false); // 8192: Do not add implied html/body elements

  ret |= getParserOption(env, props, "nocdata",
                         XML_PARSE_NOCDATA); // 16384: merge CDATA as text nodes
  ret |= getParserOption(env, props, "cdata", XML_PARSE_NOCDATA,
                         false); // 16384: merge CDATA as text nodes

  ret |= getParserOption(env, props, "noxincnode",
      XML_PARSE_NOXINCNODE); // 32768: do not generate XINCLUDE START/END nodes
  ret |=
      getParserOption(env, props, "xincnode", XML_PARSE_NOXINCNODE,
                      false); // 32768: do not generate XINCLUDE START/END nodes

  ret |= getParserOption(env, props, "compact",
      XML_PARSE_COMPACT); // 65536: compact small text nodes; no modification of
                          // the tree allowed afterwards (will possibly crash if
                          // you try to modify the tree)
  /*ret |= getParserOption(env, props, "compact", HTML_PARSE_COMPACT , false);   //
   * 65536: compact small text nodes*/

  ret |= getParserOption(env, props, "old10",
      XML_PARSE_OLD10); // 131072: parse using XML-1.0 before update 5

  ret |= getParserOption(env, props, "nobasefix",
      XML_PARSE_NOBASEFIX); // 262144: do not fixup XINCLUDE xml:base uris
  ret |= getParserOption(env, props, "basefix", XML_PARSE_NOBASEFIX,
                         false); // 262144: do not fixup XINCLUDE xml:base uris

  ret |= getParserOption(env, props, "huge",
      XML_PARSE_HUGE); // 524288: relax any hardcoded limit from the parser
  ret |= getParserOption(env, props, "oldsax",
      XML_PARSE_OLDSAX); // 1048576: parse using SAX2 interface before 2.7.0

  ret |= getParserOption(env, props, "ignore_enc",
      XML_PARSE_IGNORE_ENC); // 2097152: ignore internal document encoding hint
  /*ret |= getParserOption(env, props, "ignore_enc", HTML_PARSE_IGNORE_ENC);      //
   * 2097152: ignore internal document encoding hint*/

  ret |= getParserOption(env, props, "big_lines",
                         XML_PARSE_BIG_LINES); // 4194304: Store big lines
                                               // numbers in text PSVI field

  return (xmlParserOption)ret;
}

Napi::Value XmlDocument::FromHtml(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  Napi::Object options = info[1].As<Napi::Object>();
  Napi::Value baseUrlOpt = (options).Get(Napi::String::New(env, "baseUrl"));
  Napi::Value encodingOpt = (options).Get(Napi::String::New(env, "encoding"));
  Napi::Value excludeImpliedElementsOpt =
      (options).Get(Napi::String::New(env, "excludeImpliedElements"));

  // the base URL that will be used for this HTML parsed document
  std::string baseUrl_ = baseUrlOpt.As<Napi::String>().Utf8Value();
  const char *baseUrl = baseUrl_.c_str();
  if (!baseUrlOpt.IsString()) {
    baseUrl = NULL;
  }

  // the encoding to be used for this document
  // (leave NULL for libxml to autodetect)
  std::string encoding_ = encodingOpt.As<Napi::String>().Utf8Value();
  const char *encoding = encoding_.c_str();

  if (!encodingOpt.IsString()) {
    encoding = NULL;
  }

  Napi::Array errors = Napi::Array::New(env);
  xmlResetLastError();
  xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
                            XmlSyntaxError::PushToArray);

  int opts = (int)getParserOptions(env, options);
  if (excludeImpliedElementsOpt.As<Napi::Boolean>().Value())
    opts |= HTML_PARSE_NOIMPLIED | HTML_PARSE_NODEFDTD;

  htmlDocPtr doc;
  if (!info[0].IsBuffer()) {
    // Parse a string
    std::string str = info[0].As<Napi::String>().Utf8Value();
    doc = htmlReadMemory(str.c_str(), str.length(), baseUrl, encoding, opts);
  } else {
    // Parse a buffer
    Napi::Object buf = info[0].As<Napi::Object>();
    doc = htmlReadMemory(buf.As<Napi::Buffer<char>>().Data(),
                         buf.As<Napi::Buffer<char>>().Length(), baseUrl,
                         encoding, opts);
  }

  xmlSetStructuredErrorFunc(NULL, NULL);

  if (!doc) {
    xmlError *error = xmlGetLastError();
    if (error) {
      XmlSyntaxError::BuildSyntaxError(error).As<Napi::Error>().ThrowAsJavaScriptException();
      return env.Null();
    }
    Napi::Error::New(env, "Could not parse XML string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Object doc_handle = XmlDocument::New(env, doc);
  doc_handle.As<Napi::Object>().Set(Napi::String::New(env, "errors"), errors);

  // create the xml document handle to return
  return doc_handle;
}

// FIXME: this method is almost identical to FromHtml above.
// The two should be refactored to use a common function for most
// of the work
Napi::Value XmlDocument::FromXml(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  Napi::Array errors = Napi::Array::New(env);
  xmlResetLastError();
  xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
                            XmlSyntaxError::PushToArray);

  Napi::Object options = info[1].As<Napi::Object>();
  Napi::Value baseUrlOpt = (options).Get(Napi::String::New(env, "baseUrl"));
  Napi::Value encodingOpt = (options).Get(Napi::String::New(env, "encoding"));

  // the base URL that will be used for this document
  std::string baseUrl_ = baseUrlOpt.As<Napi::String>();
  const char *baseUrl = baseUrl_.c_str();
  if (!baseUrlOpt.IsString()) {
    baseUrl = NULL;
  }

  // the encoding to be used for this document
  // (leave NULL for libxml to autodetect)
  std::string encoding_ = encodingOpt.As<Napi::String>();
  const char *encoding = encoding_.c_str();
  if (!encodingOpt.IsString()) {
    encoding = NULL;
  }

  int opts = (int)getParserOptions(env, options);
  xmlDocPtr doc;
  if (!info[0].IsBuffer()) {
    // Parse a string
    std::string str = info[0].As<Napi::String>().Utf8Value();
    doc = xmlReadMemory(str.c_str(), str.length(), baseUrl, "UTF-8", opts);
  } else {
    // Parse a buffer
    Napi::Object buf = info[0].As<Napi::Object>();
    doc = xmlReadMemory(buf.As<Napi::Buffer<char>>().Data(),
                        buf.As<Napi::Buffer<char>>().Length(), baseUrl,
                        encoding, opts);
  }

  xmlSetStructuredErrorFunc(NULL, NULL);

  if (!doc) {
    xmlError *error = xmlGetLastError();
    if (error) {
      XmlSyntaxError::BuildSyntaxError(error).As<Napi::Error>().ThrowAsJavaScriptException();
      return env.Null();
    }
    Napi::Error::New(env, "Could not parse XML string")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (opts & XML_PARSE_XINCLUDE) {
    xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
                              XmlSyntaxError::PushToArray);
    int ret = xmlXIncludeProcessFlags(doc, opts);
    xmlSetStructuredErrorFunc(NULL, NULL);

    if (ret < 0) {
      xmlError *error = xmlGetLastError();
      if (error) {
        XmlSyntaxError::BuildSyntaxError(error).As<Napi::Error>().ThrowAsJavaScriptException();
        return env.Null();
      }
      Napi::Error::New(env, "Could not perform XInclude substitution")
          .ThrowAsJavaScriptException();
      return env.Null();
    }
  }

  Napi::Object doc_handle = XmlDocument::New(env, doc);
  doc_handle.As<Napi::Object>().Set(Napi::String::New(env, "errors"), errors);

  xmlNode *root_node = xmlDocGetRootElement(doc);
  if (root_node == NULL) {
    Napi::Error::New(env, "parsed document has no root element")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  // create the xml document handle to return
  return doc_handle;
}

Napi::Value XmlDocument::Validate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass xsd").ThrowAsJavaScriptException();
    return env.Null();
  }
Napi::Array errors = Napi::Array::New(env);
  xmlResetLastError();
  xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
                            XmlSyntaxError::PushToArray);

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].As<Napi::Object>());

  xmlSchemaParserCtxtPtr parser_ctxt =
      xmlSchemaNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env, "Could not create context for schema parser")
        .ThrowAsJavaScriptException();
    return env.Null();
  }
  xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid XSD schema").ThrowAsJavaScriptException();
    return env.Null();
  }
  xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
  if (valid_ctxt == NULL) {
    Napi::Error::New(env, "Unable to create a validation context for the schema").ThrowAsJavaScriptException();
    return env.Null();
  }
  bool valid = xmlSchemaValidateDoc(valid_ctxt, document->xml_obj) == 0;

  xmlSetStructuredErrorFunc(NULL, NULL);
  info.This().As<Napi::Object>().Set(Napi::String::New(env, "validationErrors"), errors);

  xmlSchemaFreeValidCtxt(valid_ctxt);
  xmlSchemaFree(schema);
  xmlSchemaFreeParserCtxt(parser_ctxt);

  return Napi::Boolean::New(env, valid);
}

Napi::Value XmlDocument::RngValidate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass xsd").ThrowAsJavaScriptException();
    return env.Null();
  }

Napi::Array errors = Napi::Array::New(env);
  xmlResetLastError();
  xmlSetStructuredErrorFunc(reinterpret_cast<void *>(&errors),
                            XmlSyntaxError::PushToArray);

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].As<Napi::Object>());

  xmlRelaxNGParserCtxtPtr parser_ctxt =
      xmlRelaxNGNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env, "Could not create context for RELAX NG schema parser").ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlRelaxNGPtr schema = xmlRelaxNGParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid RELAX NG schema")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlRelaxNGValidCtxtPtr valid_ctxt = xmlRelaxNGNewValidCtxt(schema);
  if (valid_ctxt == NULL) {
    Napi::Error::New(env, "Unable to create a validation context for the RELAX NG schema").ThrowAsJavaScriptException();
    return env.Null();
  }
  bool valid = xmlRelaxNGValidateDoc(valid_ctxt, document->xml_obj) == 0;

  xmlSetStructuredErrorFunc(NULL, NULL);
  info.This().As<Napi::Object>().Set(Napi::String::New(env, "validationErrors"), errors);

  xmlRelaxNGFreeValidCtxt(valid_ctxt);
  xmlRelaxNGFree(schema);
  xmlRelaxNGFreeParserCtxt(parser_ctxt);

  return Napi::Boolean::New(env, valid);
}

Napi::Value XmlDocument::SchematronValidate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass schema").ThrowAsJavaScriptException();
    return env.Null();
  }

Napi::Array errors = Napi::Array::New(env);
  xmlResetLastError();

  XmlDocument *document = Napi::ObjectWrap<XmlDocument>::Unwrap(info.This().As<Napi::Object>());
  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].As<Napi::Object>());

  xmlSchematronParserCtxtPtr parser_ctxt =
      xmlSchematronNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env, "Could not create context for Schematron schema parser").ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlSchematronPtr schema = xmlSchematronParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid Schematron schema")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  xmlSchematronValidCtxtPtr valid_ctxt =
      xmlSchematronNewValidCtxt(schema, XML_SCHEMATRON_OUT_ERROR);
  if (valid_ctxt == NULL) {
    Napi::Error::New(env, "Unable to create a validation context for the Schematron schema").ThrowAsJavaScriptException();
    return env.Null();
  }
  xmlSchematronSetValidStructuredErrors(valid_ctxt, XmlSyntaxError::PushToArray,
                                        reinterpret_cast<void *>(&errors));

  bool valid = xmlSchematronValidateDoc(valid_ctxt, document->xml_obj) == 0;

  xmlSchematronSetValidStructuredErrors(valid_ctxt, NULL, NULL);
  info.This().As<Napi::Object>().Set(Napi::String::New(env, "validationErrors"), errors);

  xmlSchematronFreeValidCtxt(valid_ctxt);
  xmlSchematronFree(schema);
  xmlSchematronFreeParserCtxt(parser_ctxt);

  return Napi::Boolean::New(env, valid);
}

/// this is a blank object with prototype methods
/// this is a blank object with prototype methods
/// not exposed to the user and not called from js
Napi::Value XmlDocument::New(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  
  std::string version_str = (info.Length() > 0 && info[0].IsString())
                            ? info[0].As<Napi::String>().Utf8Value()
                            : "1.0";
  xmlDoc *doc = xmlNewDoc((const xmlChar *)(version_str.c_str()));

  std::string encoding_str = (info.Length() > 1 && info[1].IsString())
                             ? info[1].As<Napi::String>().Utf8Value()
                             : "utf8";

  XmlDocument *document = new XmlDocument(info);
  document->xml_obj = doc;
  doc->_private = document;
  document->setEncoding(encoding_str.c_str());

  return env.Undefined();
}

XmlDocument::XmlDocument(const Napi::CallbackInfo &info) 
    : Napi::ObjectWrap<XmlDocument>(info), xml_obj(nullptr) {
}

XmlDocument::~XmlDocument() {
  if (xml_obj) {
    xml_obj->_private = NULL;
    xmlFreeDoc(xml_obj);
  }
}

void XmlDocument::Initialize(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Document", {});
  
  Napi::Object proto = func.Get("prototype").As<Napi::Object>();
  proto.Set("root", Napi::Function::New(env, XmlDocument::Root));
  proto.Set("version", Napi::Function::New(env, XmlDocument::Version));
  proto.Set("encoding", Napi::Function::New(env, XmlDocument::Encoding));
  proto.Set("toString", Napi::Function::New(env, XmlDocument::ToString));
  proto.Set("validate", Napi::Function::New(env, XmlDocument::Validate));
  proto.Set("rngValidate", Napi::Function::New(env, XmlDocument::RngValidate));
  proto.Set("schematronValidate", Napi::Function::New(env, XmlDocument::SchematronValidate));
  proto.Set("_setDtd", Napi::Function::New(env, XmlDocument::SetDtd));
  proto.Set("getDtd", Napi::Function::New(env, XmlDocument::GetDtd));
  proto.Set("type", Napi::Function::New(env, XmlDocument::type));

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  target.Set(Napi::String::New(env, "fromXml"),
             Napi::Function::New(env, XmlDocument::FromXml));
  target.Set(Napi::String::New(env, "fromHtml"),
             Napi::Function::New(env, XmlDocument::FromHtml));

  // used to create new document handles
  target.Set(Napi::String::New(env, "Document"), func);

  XmlNode::Initialize(env, target);
  XmlNamespace::Initialize(env, target);
}
} // namespace libxmljs
