// Copyright 2009, Squish Tech, LLC.

#include <napi.h>
#include <uv.h>

#include <libxml/parserInternals.h>

#include "libxmljs.h"

#include "xml_sax_parser.h"

libxmljs::XmlSaxParser *LXJS_GET_PARSER_FROM_CONTEXT(void *context) {
  _xmlParserCtxt *the_context = static_cast<_xmlParserCtxt *>(context);
  return static_cast<libxmljs::XmlSaxParser *>(the_context->_private);
}

#define EMIT_SYMBOL_STRING "emit"

using namespace Napi;
Napi::Persistent<String> emit_symbol;

namespace libxmljs {

XmlSaxParser::XmlSaxParser() : context_(NULL) {
  xmlSAXHandler tmp = {
      0, // internalSubset;
      0, // isStandalone;
      0, // hasInternalSubset;
      0, // hasExternalSubset;
      0, // resolveEntity;
      0, // getEntity;
      0, // entityDecl;
      0, // notationDecl;
      0, // attributeDecl;
      0, // elementDecl;
      0, // unparsedEntityDecl;
      0, // setDocumentLocator;
      XmlSaxParser::start_document,
      XmlSaxParser::end_document, // endDocument;
      0,                          // startElement;
      0,                          // endElement;
      0,                          // reference;
      XmlSaxParser::characters,   // characters;
      0,                          // ignorableWhitespace;
      0,                          // processingInstruction;
      XmlSaxParser::comment,      // comment;
      XmlSaxParser::warning,      // warning;
      XmlSaxParser::error,        // error;
      0, // fatalError; /* unused error() get all the errors */
      0, // getParameterEntity;
      XmlSaxParser::cdata_block,      // cdataBlock;
      0,                              // externalSubset;
      XML_SAX2_MAGIC,                 /* force SAX2 */
      this,                           /* _private */
      XmlSaxParser::start_element_ns, // startElementNs;
      XmlSaxParser::end_element_ns,   // endElementNs;
      0 // SaxParserCallback::structured_error // serror
  };

  sax_handler_ = tmp;
}

XmlSaxParser::~XmlSaxParser() { this->releaseContext(); }

void XmlSaxParser::initializeContext() {
  assert(context_);
  context_->validate = 0;
  context_->_private = this;
}

void XmlSaxParser::releaseContext() {
  if (context_) {
    context_->_private = 0;
    if (context_->myDoc != NULL) {
      xmlFreeDoc(context_->myDoc);
      context_->myDoc = NULL;
    }

    xmlFreeParserCtxt(context_);
    context_ = 0;
  }
}

Napi::Value XmlSaxParser::NewParser(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlSaxParser *parser = new XmlSaxParser();
  parser->Wrap(info.This());

  return return info.This();
}

Napi::Value XmlSaxParser::NewPushParser(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlSaxParser *parser = new XmlSaxParser();
  parser->initialize_push_parser();
  parser->Wrap(info.This());

  return return info.This();
}

void XmlSaxParser::Callback(const char *what, int argc, Napi::Value argv[]) {
  Napi::HandleScope scope(env);

  // new arguments array with first argument being the event name
  Napi::Value *args = new Napi::Value[argc + 1];
  args[0] = Napi::String::New(env, what);
  for (int i = 1; i <= argc; ++i) {
    args[i] = argv[i - 1];
  }

  // get the 'emit' function from ourselves
  Napi::Value emit_v = (this->handle()).Get(Napi::New(env, emit_symbol));
  assert(emit_v->IsFunction());

  // trigger the event
  Napi::AsyncResource res("nan:makeCallback");
  res.runInAsyncScope(this->handle(), emit_v.As<Napi::Function>(), argc + 1,
                      args);

  delete[] args;
}

Napi::Value XmlSaxParser::Push(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  LIBXMLJS_ARGUMENT_TYPE_CHECK(info[0], IsString,
                               "Bad Argument: parseString requires a string");

  XmlSaxParser *parser = info.This().Unwrap<XmlSaxParser>();

  std::string parsable = info[0].As<Napi::String>(.To<Napi::String>());

  bool terminate =
      info.Length() > 1 ? info[1].To<Napi::Boolean>()->Value() : false;

  parser->push(*parsable, parsable.Length(), terminate);

  return return env.True();
}

void XmlSaxParser::initialize_push_parser() {
  context_ = xmlCreatePushParserCtxt(&sax_handler_, NULL, NULL, 0, "");
  context_->replaceEntities = 1;
  initializeContext();
}

void XmlSaxParser::push(const char *str, unsigned int size,
                        bool terminate = false) {
  xmlParseChunk(context_, str, size, terminate);
}

Napi::Value XmlSaxParser::ParseString(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  LIBXMLJS_ARGUMENT_TYPE_CHECK(info[0], IsString,
                               "Bad Argument: parseString requires a string");

  XmlSaxParser *parser = info.This().Unwrap<XmlSaxParser>();

  std::string parsable = info[0].As<Napi::String>();
  parser->parse_string(*parsable, parsable.Length());

  // TODO(sprsquish): return based on the parser
  return return env.True();
}

void XmlSaxParser::parse_string(const char *str, unsigned int size) {
  context_ = xmlCreateMemoryParserCtxt(str, size);
  initializeContext();
  context_->replaceEntities = 1;
  xmlSAXHandler *old_sax = context_->sax;
  context_->sax = &sax_handler_;
  xmlParseDocument(context_);
  context_->sax = old_sax;
  releaseContext();
}

void XmlSaxParser::start_document(void *context) {
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  parser->Callback("startDocument");
}

void XmlSaxParser::end_document(void *context) {
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  parser->Callback("endDocument");
}

void XmlSaxParser::start_element_ns(void *context, const xmlChar *localname,
                                    const xmlChar *prefix, const xmlChar *uri,
                                    int nb_namespaces,
                                    const xmlChar **namespaces,
                                    int nb_attributes, int nb_defaulted,
                                    const xmlChar **attributes) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);

  const int argc = 5;
  const xmlChar *nsPref, *nsUri, *attrLocal, *attrPref, *attrUri, *attrVal;
  int i, j;

  Napi::Array elem;

  // Initialize argv with localname, prefix, and uri
  Napi::Value argv[argc] = {Napi::String::New(env, (const char *)localname)};

  // Build attributes list
  // Each attribute is an array of [localname, prefix, URI, value, end]
  Napi::Array attrList = Napi::Array::New(env, nb_attributes);
  if (attributes) {
    for (i = 0, j = 0; j < nb_attributes; i += 5, j++) {
      attrLocal = attributes[i + 0];
      attrPref = attributes[i + 1];
      attrUri = attributes[i + 2];
      attrVal = attributes[i + 3];

      elem = Napi::Array::New(env, 4);

      (elem).Set(Napi::Number::New(env, 0),
                 Napi::String::New(env, (const char *)attrLocal,
                                   xmlStrlen(attrLocal)));

      (elem).Set(
          Napi::Number::New(env, 1),
          Napi::String::New(env, (const char *)attrPref, xmlStrlen(attrPref)));

      (elem).Set(
          Napi::Number::New(env, 2),
          Napi::String::New(env, (const char *)attrUri, xmlStrlen(attrUri)));

      (elem).Set(Napi::Number::New(env, 3),
                 Napi::String::New(env, (const char *)attrVal,
                                   attributes[i + 4] - attrVal));

      (attrList).Set(Napi::Number::New(env, j), elem);
    }
  }
  argv[1] = attrList;

  if (prefix) {
    argv[2] = Napi::String::New(env, (const char *)prefix);
  } else {
    argv[2] = env.Null();
  }

  if (uri) {
    argv[3] = Napi::String::New(env, (const char *)uri);
  } else {
    argv[3] = env.Null();
  }

  // Build namespace array of arrays [[prefix, ns], [prefix, ns]]
  Napi::Array nsList = Napi::Array::New(env, nb_namespaces);
  if (namespaces) {
    for (i = 0, j = 0; j < nb_namespaces; j++) {
      nsPref = namespaces[i++];
      nsUri = namespaces[i++];

      elem = Napi::Array::New(env, 2);
      if (xmlStrlen(nsPref) == 0) {
        (elem).Set(Napi::Number::New(env, 0), env.Null());
      } else {
        (elem).Set(
            Napi::Number::New(env, 0),
            Napi::String::New(env, (const char *)nsPref, xmlStrlen(nsPref)));
      }

      (elem).Set(Napi::Number::New(env, 1),
                 Napi::String::New(env, (const char *)nsUri, xmlStrlen(nsUri)));

      (nsList).Set(Napi::Number::New(env, j), elem);
    }
  }
  argv[4] = nsList;

  parser->Callback("startElementNS", argc, argv);
}

void XmlSaxParser::end_element_ns(void *context, const xmlChar *localname,
                                  const xmlChar *prefix, const xmlChar *uri) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);

  Napi::Value argv[3];
  argv[0] = Napi::String::New(env, (const char *)localname);

  if (prefix) {
    argv[1] = Napi::String::New(env, (const char *)prefix);
  } else {
    argv[1] = env.Null();
  }

  if (uri) {
    argv[2] = Napi::String::New(env, (const char *)uri);
  } else {
    argv[2] = env.Null();
  }

  parser->Callback("endElementNS", 3, argv);
}

void XmlSaxParser::characters(void *context, const xmlChar *ch, int len) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);

  Napi::Value argv[1] = {Napi::String::New(env, (const char *)ch, len)};
  parser->Callback("characters", 1, argv);
}

void XmlSaxParser::comment(void *context, const xmlChar *value) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  Napi::Value argv[1] = {Napi::String::New(env, (const char *)value)};
  parser->Callback("comment", 1, argv);
}

void XmlSaxParser::cdata_block(void *context, const xmlChar *value, int len) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);
  Napi::Value argv[1] = {Napi::String::New(env, (const char *)value, len)};
  parser->Callback("cdata", 1, argv);
}

#ifdef WIN32
// https://github.com/json-c/json-c/blob/master/printbuf.c
// Copyright (c) 2004, 2005 Metaparadigm Pte Ltd
static int vasprintf(char **buf, const char *fmt, va_list ap) {
  int chars;
  char *b;

  if (!buf) {
    return -1;
  }

  chars = _vscprintf(fmt, ap) + 1;

  b = (char *)malloc(sizeof(char) * chars);
  if (!b) {
    return -1;
  }

  if ((chars = vsprintf(b, fmt, ap)) < 0) {
    free(b);
  } else {
    *buf = b;
  }

  return chars;
}
#endif /* WIN32 */

void XmlSaxParser::warning(void *context, const char *msg, ...) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);

  char *message;

  va_list args;
  va_start(args, msg);
  if (vasprintf(&message, msg, args) >= 0) {
    Napi::Value argv[1] = {Napi::String::New(env, (const char *)message)};
    parser->Callback("warning", 1, argv);
  }

  va_end(args);
  free(message);
}

void XmlSaxParser::error(void *context, const char *msg, ...) {
  Napi::HandleScope scope(env);
  libxmljs::XmlSaxParser *parser = LXJS_GET_PARSER_FROM_CONTEXT(context);

  char *message;

  va_list args;
  va_start(args, msg);
  if (vasprintf(&message, msg, args) >= 0) {
    Napi::Value argv[1] = {Napi::String::New(env, (const char *)message)};
    parser->Callback("error", 1, argv);
  }

  va_end(args);
  free(message);
}

void XmlSaxParser::Initialize(Napi::Env env, Napi::Object target) {
  Napi::HandleScope scope(env);

  emit_symbol.Reset(Napi::String::New(env, EMIT_SYMBOL_STRING));

  // SAX Parser
  Napi::FunctionReference parser_t = Napi::Function::New(env, NewParser);

  Napi::FunctionReference sax_parser_template;
  sax_parser_template.Reset(parser_t);

  Napi::SetPrototypeMethod(parser_t, "parseString", XmlSaxParser::ParseString);

  (target).Set(Napi::String::New(env, "SaxParser"),
               Napi::GetFunction(parser_t));

  Napi::FunctionReference push_parser_t =
      Napi::Function::New(env, NewPushParser);

  // Push Parser
  Napi::FunctionReference sax_push_parser_template;
  sax_push_parser_template.Reset(push_parser_t);

  Napi::SetPrototypeMethod(push_parser_t, "push", XmlSaxParser::Push);

  (target).Set(Napi::String::New(env, "SaxPushParser"),
               Napi::GetFunction(push_parser_t));
}
} // namespace libxmljs
