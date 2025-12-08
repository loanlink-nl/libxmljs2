// Copyright 2009, Squish Tech, LLC.

#include <cstdio>
#include <cstring>

// #include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/relaxng.h>
#include <libxml/schematron.h>
#include <libxml/xinclude.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlschemas.h>

#include "napi.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_syntax_error.h"

namespace libxmljs {

Napi::FunctionReference XmlDocument::constructor;

// JS-signature: (version?: string, encoding?: string)
XmlDocument::XmlDocument(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<XmlDocument>(info) {

  if (info.Length() > 0 && info[0].IsExternal()) {
    xml_obj = info[0].As<Napi::External<xmlDoc>>().Data();
    xml_obj->_private = this;
    return;
  }

  const std::string version = info.Length() > 0 && info[0].IsString()
                                  ? info[0].ToString().Utf8Value()
                                  : "1.0";
  xml_obj = xmlNewDoc((const xmlChar *)(version.c_str()));
  xml_obj->_private = this;

  const std::string encoding = info.Length() > 1 && info[1].IsString()
                                   ? info[1].ToString().Utf8Value()
                                   : "utf8";

  this->setEncoding(encoding);
}

XmlDocument::~XmlDocument() {
  this->xml_obj->_private = NULL;
  xmlFreeDoc(this->xml_obj);
}

Napi::Value XmlDocument::Encoding(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  // if no args, get the encoding
  if (info.Length() == 0 || info[0].IsUndefined()) {
    if (xml_obj->encoding) {
      return scope.Escape(
          Napi::String::New(env, (const char *)xml_obj->encoding,
                            xmlStrlen((const xmlChar *)xml_obj->encoding)));
    }
    return scope.Escape(env.Null());
  }

  // set the encoding otherwise
  std::string encoding = info[0].ToString().Utf8Value();
  this->setEncoding(encoding);
  return info.This();
}

void XmlDocument::setEncoding(const std::string encoding) {
  if (xml_obj->encoding != NULL) {
    xmlFree((xmlChar *)xml_obj->encoding);
  }

  xml_obj->encoding = xmlStrdup((const xmlChar *)encoding.c_str());
}

Napi::Value XmlDocument::Version(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (xml_obj->version) {
    return scope.Escape(
        Napi::String::New(env, (const char *)xml_obj->version,
                          xmlStrlen((const xmlChar *)xml_obj->version)));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlDocument::Root(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  xmlNode *root = xmlDocGetRootElement(this->xml_obj);

  if (info.Length() == 0 || info[0].IsUndefined()) {
    if (!root) {
      return env.Null();
    }
    return scope.Escape(XmlElement::NewInstance(env, root));
  }

  if (root != NULL) {
    Napi::Error::New(env, "Holder document already has a root node")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // set the element as the root element for the document
  // allows for proper retrieval of root later
  XmlElement *element = XmlElement::Unwrap(info[0].ToObject());
  xmlDocSetRootElement(this->xml_obj, element->xml_obj);
  element->ref_wrapped_ancestor();
  return scope.Escape(info[0]);
}

Napi::Value XmlDocument::GetDtd(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  xmlDtdPtr dtd = xmlGetIntSubset(xml_obj);

  if (!dtd) {
    return scope.Escape(env.Null());
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

  dtdObj.Set("name", nameValue);
  dtdObj.Set("externalId", extValue);
  dtdObj.Set("systemId", sysValue);

  return scope.Escape(dtdObj);
}

Napi::Value XmlDocument::SetDtd(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  std::string name = info[0].ToString().Utf8Value();

  const char *extId = NULL;
  const char *sysId = NULL;

  std::string extIdStr;
  std::string sysIdStr;

  if (info.Length() > 1 && info[1].IsString()) {
    extIdStr = info[1].ToString().Utf8Value();
    if (!extIdStr.empty()) {
      extId = extIdStr.c_str();
    }
  }
  if (info.Length() > 2 && info[2].IsString()) {
    sysIdStr = info[2].ToString().Utf8Value();
    if (!sysIdStr.empty()) {
      sysId = sysIdStr.c_str();
    }
  }

  // No good way of unsetting the doctype if it is previously set...this allows
  // us to.
  xmlDtdPtr dtd = xmlGetIntSubset(this->xml_obj);

  xmlUnlinkNode((xmlNodePtr)dtd);
  xmlFreeNode((xmlNodePtr)dtd);

  xmlCreateIntSubset(this->xml_obj, (const xmlChar *)name.c_str(),
                     (const xmlChar *)extId, (const xmlChar *)sysId);

  return info.This();
}

Napi::Value XmlDocument::ToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  int options = 0;
  std::string encoding = "UTF-8";

  if (info[0].IsObject()) {
    Napi::Object obj = info[0].ToObject();

    // drop the xml declaration
    if (obj.Has("declaration") &&
        obj.Get("declaration").ToBoolean().Value() == false) {
      options |= XML_SAVE_NO_DECL;
    }

    // format save output
    if (obj.Has("format") && obj.Get("format").ToBoolean().Value() == true) {
      options |= XML_SAVE_FORMAT;
    }

    // no empty tags (only works with XML) ex: <title></title> becomes <title/>
    if (obj.Has("selfCloseEmpty") &&
        obj.Get("selfCloseEmpty").ToBoolean().Value() == false) {
      options |= XML_SAVE_NO_EMPTY;
    }

    // format with non-significant whitespace
    if (obj.Has("whitespace") &&
        obj.Get("whitespace").ToBoolean().Value() == true) {
      options |= XML_SAVE_WSNONSIG;
    }

    if (obj.Has("encoding") && obj.Get("encoding").IsString()) {
      encoding = obj.Get("encoding").ToString().Utf8Value();
    }

    if (obj.Has("type")) {
      Napi::Value type = obj.Get("type");
      std::string typeStr;
      if (type.IsString()) {
        typeStr = type.ToString().Utf8Value();
      }

      if (typeStr == "XML" || typeStr == "xml") {
        options |= XML_SAVE_AS_XML; // force XML serialization on HTML doc
      } else if (typeStr == "HTML" || typeStr == "html") {
        options |= XML_SAVE_AS_HTML; // force HTML serialization on XML doc
        // if the document is XML and we want formatted HTML output
        // we must use the XHTML serializer because the default HTML
        // serializer only formats node->type = HTML_NODE and not XML_NODEs
        if ((options & XML_SAVE_FORMAT) &&
            (options & XML_SAVE_XHTML) == false) {
          options |= XML_SAVE_XHTML;
        }
      } else if (typeStr == "XHTML" || typeStr == "xhtml") {
        options |= XML_SAVE_XHTML; // force XHTML serialization
      }
    }
  } else if (info.Length() == 0 || info[0].ToBoolean().Value() == true) {
    options |= XML_SAVE_FORMAT;
  }

  xmlBuffer *buf = xmlBufferCreate();
  xmlSaveCtxt *savectx = xmlSaveToBuffer(buf, encoding.c_str(), options);
  xmlSaveTree(savectx, (xmlNode *)xml_obj);
  xmlSaveFlush(savectx);
  xmlSaveClose(savectx);
  Napi::Value ret = env.Null();
  if (xmlBufferLength(buf) > 0) {
    ret = Napi::String::New(env, (char *)xmlBufferContent(buf),
                            xmlBufferLength(buf));
  }
  xmlBufferFree(buf);

  return scope.Escape(ret);
}

Napi::Value XmlDocument::Type(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "document");
}

// not called from node
// private api
Napi::Value XmlDocument::NewInstance(Napi::Env env, xmlDoc *doc) {
  Napi::EscapableHandleScope scope(env);

  if (doc->_private) {
    auto instance = static_cast<XmlDocument *>(doc->_private)->Value();
    if (!instance.IsEmpty()) {
      return scope.Escape(instance);
    }
  }

  auto external = Napi::External<xmlDoc>::New(env, doc);
  Napi::Object obj = constructor.New({external});

  return scope.Escape(obj);
}

int getParserOption(Napi::Object props, const char *key, int value,
                    bool defaultValue = true) {
  if (!props.Has(key)) {
    return 0;
  }
  Napi::Value prop = props.Get(key);
  return !prop.IsUndefined() && prop.ToBoolean().Value() == defaultValue ? value
                                                                         : 0;
}

xmlParserOption getParserOptions(Napi::Object props) {
  int ret = 0;

  // http://xmlsoft.org/html/libxml-parser.html#xmlParserOption
  // http://www.xmlsoft.org/html/libxml-HTMLparser.html#htmlParserOption

  ret |= getParserOption(props, "recover",
                         XML_PARSE_RECOVER); // 1: recover on errors
  /*ret |= getParserOption(props, "recover", HTML_PARSE_RECOVER);           //
   * 1: Relaxed parsing*/

  ret |= getParserOption(props, "noent",
                         XML_PARSE_NOENT); // 2: substitute entities

  ret |= getParserOption(props, "dtdload",
                         XML_PARSE_DTDLOAD); // 4: load the external subset
  ret |= getParserOption(props, "doctype", HTML_PARSE_NODEFDTD,
                         false); // 4: do not default a doctype if not found

  ret |= getParserOption(props, "dtdattr",
                         XML_PARSE_DTDATTR); // 8: default DTD attributes
  ret |= getParserOption(props, "dtdvalid",
                         XML_PARSE_DTDVALID); // 16: validate with the DTD

  ret |= getParserOption(props, "noerror",
                         XML_PARSE_NOERROR); // 32: suppress error reports
  ret |= getParserOption(props, "errors", HTML_PARSE_NOERROR,
                         false); // 32: suppress error reports

  ret |= getParserOption(props, "nowarning",
                         XML_PARSE_NOWARNING); // 64: suppress warning reports
  ret |= getParserOption(props, "warnings", HTML_PARSE_NOWARNING,
                         false); // 64: suppress warning reports

  ret |= getParserOption(props, "pedantic",
                         XML_PARSE_PEDANTIC); // 128: pedantic error reporting
  /*ret |= getParserOption(props, "pedantic", HTML_PARSE_PEDANTIC);         //
   * 128: pedantic error reporting*/

  ret |= getParserOption(props, "noblanks",
                         XML_PARSE_NOBLANKS); // 256: remove blank nodes
  ret |= getParserOption(props, "blanks", HTML_PARSE_NOBLANKS,
                         false); // 256: remove blank nodes

  ret |= getParserOption(
      props, "sax1", XML_PARSE_SAX1); // 512: use the SAX1 interface internally
  ret |= getParserOption(
      props, "xinclude",
      XML_PARSE_XINCLUDE); // 1024: Implement XInclude substitition

  ret |= getParserOption(props, "nonet",
                         XML_PARSE_NONET); // 2048: Forbid network access
  ret |= getParserOption(props, "net", HTML_PARSE_NONET,
                         false); // 2048: Forbid network access

  ret |= getParserOption(
      props, "nodict",
      XML_PARSE_NODICT); // 4096: Do not reuse the context dictionnary
  ret |= getParserOption(props, "dict", XML_PARSE_NODICT,
                         false); // 4096: Do not reuse the context dictionnary

  ret |= getParserOption(
      props, "nsclean",
      XML_PARSE_NSCLEAN); // 8192: remove redundant namespaces declarations
  ret |= getParserOption(props, "implied", HTML_PARSE_NOIMPLIED,
                         false); // 8192: Do not add implied html/body elements

  ret |= getParserOption(props, "nocdata",
                         XML_PARSE_NOCDATA); // 16384: merge CDATA as text nodes
  ret |= getParserOption(props, "cdata", XML_PARSE_NOCDATA,
                         false); // 16384: merge CDATA as text nodes

  ret |= getParserOption(
      props, "noxincnode",
      XML_PARSE_NOXINCNODE); // 32768: do not generate XINCLUDE START/END nodes
  ret |=
      getParserOption(props, "xincnode", XML_PARSE_NOXINCNODE,
                      false); // 32768: do not generate XINCLUDE START/END nodes

  ret |= getParserOption(
      props, "compact",
      XML_PARSE_COMPACT); // 65536: compact small text nodes; no modification of
                          // the tree allowed afterwards (will possibly crash if
                          // you try to modify the tree)
  /*ret |= getParserOption(props, "compact", HTML_PARSE_COMPACT , false);   //
   * 65536: compact small text nodes*/

  ret |= getParserOption(
      props, "old10",
      XML_PARSE_OLD10); // 131072: parse using XML-1.0 before update 5

  ret |= getParserOption(
      props, "nobasefix",
      XML_PARSE_NOBASEFIX); // 262144: do not fixup XINCLUDE xml:base uris
  ret |= getParserOption(props, "basefix", XML_PARSE_NOBASEFIX,
                         false); // 262144: do not fixup XINCLUDE xml:base uris

  ret |= getParserOption(
      props, "huge",
      XML_PARSE_HUGE); // 524288: relax any hardcoded limit from the parser
  ret |= getParserOption(
      props, "oldsax",
      XML_PARSE_OLDSAX); // 1048576: parse using SAX2 interface before 2.7.0

  ret |= getParserOption(
      props, "ignore_enc",
      XML_PARSE_IGNORE_ENC); // 2097152: ignore internal document encoding hint
  /*ret |= getParserOption(props, "ignore_enc", HTML_PARSE_IGNORE_ENC);      //
   * 2097152: ignore internal document encoding hint*/

  ret |= getParserOption(props, "big_lines",
                         XML_PARSE_BIG_LINES); // 4194304: Store big lines
                                               // numbers in text PSVI field

  return (xmlParserOption)ret;
}

Napi::Value XmlDocument::FromHtml(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  Napi::Object options = info[1].ToObject();

  // the base URL that will be used for this HTML parsed document
  const char *baseUrl = NULL;
  std::string baseUrlStr;
  if (options.Has("baseUrl") && options.Get("baseUrl").IsString()) {
    baseUrlStr = options.Get("baseUrl").ToString().Utf8Value();
    baseUrl = baseUrlStr.c_str();
  }

  // the encoding to be used for this document
  // (leave NULL for libxml to autodetect)
  const char *encoding = NULL;
  std::string encodingStr;
  if (options.Has("encoding") && options.Get("encoding").IsString()) {
    encodingStr = options.Get("encoding").ToString().Utf8Value();
    encoding = encodingStr.c_str();
  }

  ErrorArrayContext ctx{env, Napi::Array::New(env)};

  xmlResetLastError();
  xmlSetStructuredErrorFunc(&ctx, XmlSyntaxError::PushToArray);

  int opts = (int)getParserOptions(options);
  if (options.Has("excludeImpliedElements") &&
      options.Get("excludeImpliedElements").ToBoolean().Value()) {
    opts |= HTML_PARSE_NOIMPLIED | HTML_PARSE_NODEFDTD;
  }

  htmlDocPtr doc;
  if (!info[0].IsBuffer()) {
    // Parse a string
    std::string str = info[0].ToString().Utf8Value();
    doc = htmlReadMemory(str.c_str(), str.length(), baseUrl, encoding, opts);
  } else {
    // Parse a buffer
    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    doc = htmlReadMemory(buf.Data(), buf.Length(), baseUrl, encoding, opts);
  }

  xmlSetStructuredErrorFunc(NULL, NULL);

  if (!doc) {
    const xmlError *error = xmlGetLastError();
    if (error) {
      XmlSyntaxError::BuildSyntaxError(env, error).ThrowAsJavaScriptException();
      return scope.Escape(env.Undefined());
    }
    Napi::Error::New(env, "Could not parse XML string")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  Napi::Object doc_handle = XmlDocument::NewInstance(env, doc).ToObject();
  doc_handle.Set("errors", ctx.errors);

  // create the xml document handle to return
  return scope.Escape(doc_handle);
}

// FIXME: this method is almost identical to FromHtml above.
// The two should be refactored to use a common function for most
// of the work
Napi::Value XmlDocument::FromXml(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  Napi::Array errors = Napi::Array::New(env);

  ErrorArrayContext ctx{env, errors};

  xmlResetLastError();
  xmlSetStructuredErrorFunc(&ctx, XmlSyntaxError::PushToArray);
  Napi::Object options = info[1].ToObject();

  // the base URL that will be used for this document
  const char *baseUrl = NULL;
  std::string baseUrlStr;
  if (options.Has("baseUrl") && options.Get("baseUrl").IsString()) {
    baseUrlStr = options.Get("baseUrl").ToString().Utf8Value();
    baseUrl = baseUrlStr.c_str();
  }

  // the encoding to be used for this document
  // (leave NULL for libxml to autodetect)
  const char *encoding = NULL;
  std::string encodingStr;
  if (options.Has("encoding") && options.Get("encoding").IsString()) {
    encodingStr = options.Get("encoding").ToString().Utf8Value();
    encoding = encodingStr.c_str();
  }

  int opts = (int)getParserOptions(options);
  xmlDoc *doc;
  if (!info[0].IsBuffer()) {
    // Parse a string
    std::string str = info[0].ToString().Utf8Value();
    doc = xmlReadMemory(str.c_str(), str.length(), baseUrl, encoding, opts);
  } else {
    // Parse a buffer
    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    doc = xmlReadMemory(buf.Data(), buf.Length(), baseUrl, encoding, opts);
  }

  xmlSetStructuredErrorFunc(NULL, NULL);

  if (!doc) {
    const xmlError *error = xmlGetLastError();
    if (error) {
      XmlSyntaxError::BuildSyntaxError(env, error).ThrowAsJavaScriptException();
      return env.Undefined();
    }
    Napi::Error::New(env, "Could not parse XML string")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (opts & XML_PARSE_XINCLUDE) {
    xmlSetStructuredErrorFunc(&ctx, XmlSyntaxError::PushToArray);
    int ret = xmlXIncludeProcessFlags(doc, opts);
    xmlSetStructuredErrorFunc(NULL, NULL);

    if (ret < 0) {
      const xmlError *error = xmlGetLastError();
      if (error) {
        XmlSyntaxError::BuildSyntaxError(env, error)
            .ThrowAsJavaScriptException();
        return env.Undefined();
      }
      Napi::Error::New(env, "Could not perform XInclude substitution")
          .ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }

  Napi::Value x = XmlDocument::NewInstance(env, doc);
  Napi::Object doc_handle = x.ToObject();
  doc_handle.Set("errors", ctx.errors);

  xmlNode *root_node = xmlDocGetRootElement(doc);
  if (root_node == NULL) {
    Napi::Error::New(env, "parsed document has no root element")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  // create the xml document handle to return
  return scope.Escape(doc_handle);
}

Napi::Value XmlDocument::Validate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass xsd").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }
  if (!info[0].IsObject() ||
      !info[0].ToObject().InstanceOf(XmlDocument::constructor.Value())) {
    Napi::Error::New(env, "Must pass XmlDocument").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  Napi::Array errors = Napi::Array::New(env);
  ErrorArrayContext ctx{env, errors};

  xmlResetLastError();
  xmlSetStructuredErrorFunc(&ctx, XmlSyntaxError::PushToArray);

  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].ToObject());

  xmlSchemaParserCtxtPtr parser_ctxt =
      xmlSchemaNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env, "Could not create context for schema parser")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }
  xmlSchemaPtr schema = xmlSchemaParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid XSD schema").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }
  xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
  if (valid_ctxt == NULL) {
    Napi::Error::New(env,
                     "Unable to create a validation context for the schema")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  bool valid = xmlSchemaValidateDoc(valid_ctxt, xml_obj) == 0;

  xmlSetStructuredErrorFunc(NULL, NULL);
  this->Value().Set("validationErrors", errors);

  xmlSchemaFreeValidCtxt(valid_ctxt);
  xmlSchemaFree(schema);
  xmlSchemaFreeParserCtxt(parser_ctxt);

  return scope.Escape(Napi::Boolean::New(env, valid));
}

Napi::Value XmlDocument::RngValidate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass xsd").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  if (!info[0].IsObject() ||
      !info[0].ToObject().InstanceOf(constructor.Value())) {
    Napi::Error::New(env, "Must pass XmlDocument").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  Napi::Array errors = Napi::Array::New(env);
  ErrorArrayContext ctx{env, errors};

  xmlResetLastError();
  xmlSetStructuredErrorFunc(&ctx, XmlSyntaxError::PushToArray);

  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].ToObject());

  xmlRelaxNGParserCtxtPtr parser_ctxt =
      xmlRelaxNGNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env, "Could not create context for RELAX NG schema parser")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlRelaxNGPtr schema = xmlRelaxNGParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid RELAX NG schema")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlRelaxNGValidCtxtPtr valid_ctxt = xmlRelaxNGNewValidCtxt(schema);
  if (valid_ctxt == NULL) {
    Napi::Error::New(
        env, "Unable to create a validation context for the RELAX NG schema")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }
  bool valid = xmlRelaxNGValidateDoc(valid_ctxt, xml_obj) == 0;

  xmlSetStructuredErrorFunc(NULL, NULL);
  this->Value().Set("validationErrors", errors);

  xmlRelaxNGFreeValidCtxt(valid_ctxt);
  xmlRelaxNGFree(schema);
  xmlRelaxNGFreeParserCtxt(parser_ctxt);

  return scope.Escape(Napi::Boolean::New(env, valid));
}

Napi::Value XmlDocument::SchematronValidate(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0 || info[0].IsNull() || info[0].IsUndefined()) {
    Napi::Error::New(env, "Must pass schema").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  if (!info[0].IsObject() ||
      !info[0].ToObject().InstanceOf(constructor.Value())) {
    Napi::Error::New(env, "Must pass XmlDocument").ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  ErrorArrayContext ctx{env, Napi::Array::New(env)};
  xmlResetLastError();

  XmlDocument *documentSchema =
      Napi::ObjectWrap<XmlDocument>::Unwrap(info[0].ToObject());

  xmlSchematronParserCtxtPtr parser_ctxt =
      xmlSchematronNewDocParserCtxt(documentSchema->xml_obj);
  if (parser_ctxt == NULL) {
    Napi::Error::New(env,
                     "Could not create context for Schematron schema parser")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlSchematronPtr schema = xmlSchematronParse(parser_ctxt);
  if (schema == NULL) {
    Napi::Error::New(env, "Invalid Schematron schema")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }

  xmlSchematronValidCtxtPtr valid_ctxt =
      xmlSchematronNewValidCtxt(schema, XML_SCHEMATRON_OUT_ERROR);
  if (valid_ctxt == NULL) {
    Napi::Error::New(
        env, "Unable to create a validation context for the Schematron schema")
        .ThrowAsJavaScriptException();
    return scope.Escape(env.Undefined());
  }
  xmlSchematronSetValidStructuredErrors(valid_ctxt, XmlSyntaxError::PushToArray,
                                        reinterpret_cast<void *>(&ctx));

  bool valid = xmlSchematronValidateDoc(valid_ctxt, xml_obj) == 0;

  xmlSchematronSetValidStructuredErrors(valid_ctxt, NULL, NULL);
  this->Value().Set("validationErrors", ctx.errors);

  xmlSchematronFreeValidCtxt(valid_ctxt);
  xmlSchematronFree(schema);
  xmlSchematronFreeParserCtxt(parser_ctxt);

  return scope.Escape(Napi::Boolean::New(env, valid));
}

void XmlDocument::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function ctor =
      DefineClass(env, "Document",
                  {
                      InstanceMethod("root", &XmlDocument::Root),
                      InstanceMethod("version", &XmlDocument::Version),
                      InstanceMethod("encoding", &XmlDocument::Encoding),
                      InstanceMethod("toString", &XmlDocument::ToString),
                      InstanceMethod("validate", &XmlDocument::Validate),
                      InstanceMethod("rngValidate", &XmlDocument::RngValidate),
                      InstanceMethod("schematronValidate",
                                     &XmlDocument::SchematronValidate),
                      InstanceMethod("_setDtd", &XmlDocument::SetDtd),
                      InstanceMethod("getDtd", &XmlDocument::GetDtd),
                      InstanceMethod("type", &XmlDocument::Type),
                  });

  constructor = Napi::Persistent(ctor);
  constructor.SuppressDestruct();
  env.AddCleanupHook([]() { constructor.Reset(); });

  exports.Set("Document", ctor);
  exports.Set("fromXml", Napi::Function::New(env, XmlDocument::FromXml));
  exports.Set("fromHtml", Napi::Function::New(env, XmlDocument::FromHtml));

  XmlNamespace::Init(env, exports);
}
} // namespace libxmljs
