// Copyright 2009, Squish Tech, LLC.
#include <cstring>

#include "libxmljs.h"

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "xml_element.h"
#include "xml_node.h"
#include "xml_xpath_context.h"

namespace libxmljs {

XmlXpathContext::XmlXpathContext(xmlNode *node) {
  ctxt = xmlXPathNewContext(node->doc);
  ctxt->node = node;
}

XmlXpathContext::~XmlXpathContext() { xmlXPathFreeContext(ctxt); }

void XmlXpathContext::register_ns(const xmlChar *prefix, const xmlChar *uri) {
  xmlXPathRegisterNs(ctxt, prefix, uri);
}

Napi::Value XmlXpathContext::evaluate(Napi::Env env, const xmlChar *xpath) {
  xmlXPathObject *xpathobj = xmlXPathEval(xpath, ctxt);
  Napi::Value res;

  if (xpathobj) {
    switch (xpathobj->type) {
    case XPATH_NODESET: {
      if (xmlXPathNodeSetIsEmpty(xpathobj->nodesetval)) {
        res = Napi::Array::New(env, 0);
        break;
      }

      Napi::Array nodes = Napi::Array::New(env, xpathobj->nodesetval->nodeNr);
      for (int i = 0; i != xpathobj->nodesetval->nodeNr; ++i) {
        Napi::Value node_obj =
            XmlNodeInstance::NewInstance(env, xpathobj->nodesetval->nodeTab[i]);
        nodes.Set(i, node_obj);
      }

      res = nodes;
      break;
    }

    case XPATH_BOOLEAN:
      res = Napi::Boolean::New(env, xpathobj->boolval);
      break;

    case XPATH_NUMBER:
      res = Napi::Number::New(env, xpathobj->floatval);
      break;

    case XPATH_STRING:
      res = Napi::String::New(env, (const char *)xpathobj->stringval,
                              xmlStrlen(xpathobj->stringval));
      break;

    default:
      res = env.Null();
      break;
    }
  } else {
    res = env.Null();
  }

  xmlXPathFreeObject(xpathobj);
  return res;
}

} // namespace libxmljs
