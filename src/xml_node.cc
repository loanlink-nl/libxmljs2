// Copyright 2009, Squish Tech, LLC.

#include <napi.h>
#include <uv.h>

#include <libxml/xmlsave.h>

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_pi.h"
#include "xml_text.h"

using namespace Napi;

namespace libxmljs {

Napi::FunctionReference XmlNode::constructor;

Napi::Value XmlNode::Doc(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_doc();
}

Napi::Value XmlNode::Namespace(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  // #namespace() Get the node's namespace
  if (info.Length() == 0) {
    return return node->get_namespace();
  }

  if (info[0].IsNull())
    return return node->remove_namespace();

  XmlNamespace *ns = NULL;

  // #namespace(ns) libxml.Namespace object was provided
  // TODO(sprsquish): check that it was actually given a namespace obj
  if (info[0].IsObject())
    ns = Napi::ObjectWrap::Unwrap<XmlNamespace>(info[0].To<Napi::Object>());

  // #namespace(href) or #namespace(prefix, href)
  // if the namespace has already been defined on the node, just set it
  if (info[0].IsString()) {
    std::string ns_to_find = info[0].As<Napi::String>(.To<Napi::String>());
    xmlNs *found_ns = node->find_namespace(*ns_to_find);
    if (found_ns) {
      // maybe build
      Napi::Object existing = XmlNamespace::New(found_ns);
      ns = existing.Unwrap<XmlNamespace>();
    }
  }

  // Namespace does not seem to exist, so create it.
  if (!ns) {
    const unsigned int argc = 3;
    Napi::Value argv[argc];
    argv[0] = info.This();

    if (info.Length() == 1) {
      argv[1] = env.Null();
      argv[2] = info[0];
    } else {
      argv[1] = info[0];
      argv[2] = info[1];
    }

    Napi::Function define_namespace =
        Napi::GetFunction(Napi::New(env, XmlNamespace::constructor));

    // will create a new namespace attached to this node
    // since we keep the document around, the namespace, like the node, won't be
    // garbage collected
    Napi::Value new_ns = Napi::NewInstance(define_namespace, argc, argv);
    ns = Napi::ObjectWrap::Unwrap<XmlNamespace>(new_ns.To<Napi::Object>());
  }

  node->set_namespace(ns->xml_obj);
  return return info.This();
}

Napi::Value XmlNode::Namespaces(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  // ignore everything but a literal true; different from IsFalse
  if ((info.Length() == 0) || !info[0].IsTrue()) {
    return return node->get_all_namespaces();
  }

  return return node->get_local_namespaces();
}

Napi::Value XmlNode::Parent(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_parent();
}

Napi::Value XmlNode::PrevSibling(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_prev_sibling();
}

Napi::Value XmlNode::NextSibling(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_next_sibling();
}

Napi::Value XmlNode::LineNumber(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_line_number();
}

Napi::Value XmlNode::Type(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  return return node->get_type();
}

Napi::Value XmlNode::ToString(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  int options = 0;

  if (info.Length() > 0) {
    if (info[0].IsBoolean()) {
      if (info[0].IsTrue()) {
        options |= XML_SAVE_FORMAT;
      }
    } else if (info[0].IsObject()) {
      Napi::Object obj = info[0].To<Napi::Object>();

      // drop the xml declaration
      if ((obj)
              .Get(Napi::String::New(env, "declaration"))

              ->IsFalse()) {
        options |= XML_SAVE_NO_DECL;
      }

      // format save output
      if ((obj)
              .Get(Napi::String::New(env, "format"))

              ->IsTrue()) {
        options |= XML_SAVE_FORMAT;
      }

      // no empty tags (only works with XML) ex: <title></title> becomes
      // <title/>
      if ((obj)
              .Get(Napi::String::New(env, "selfCloseEmpty"))

              ->IsFalse()) {
        options |= XML_SAVE_NO_EMPTY;
      }

      // format with non-significant whitespace
      if ((obj)
              .Get(Napi::String::New(env, "whitespace"))

              ->IsTrue()) {
        options |= XML_SAVE_WSNONSIG;
      }

      Napi::Value type = (obj).Get(Napi::String::New(env, "type"));
      if (type.StrictEquals(Napi::String::New(env, "XML")).ToChecked() ||
          type.StrictEquals(Napi::String::New(env, "xml")).ToChecked()) {
        options |= XML_SAVE_AS_XML; // force XML serialization on HTML doc
      } else if (type.StrictEquals(Napi::String::New(env, "HTML"))
                     .ToChecked() ||
                 type.StrictEquals(Napi::String::New(env, "html"))
                     .ToChecked()) {
        options |= XML_SAVE_AS_HTML; // force HTML serialization on XML doc
        // if the document is XML and we want formatted HTML output
        // we must use the XHTML serializer because the default HTML
        // serializer only formats node->type = HTML_NODE and not XML_NODEs
        if ((options & XML_SAVE_FORMAT) &&
            (options & XML_SAVE_XHTML) == false) {
          options |= XML_SAVE_XHTML;
        }
      } else if (type.StrictEquals(Napi::String::New(env, "XHTML"))
                     .ToChecked() ||
                 type.StrictEquals(Napi::String::New(env, "xhtml"))
                     .ToChecked()) {
        options |= XML_SAVE_XHTML; // force XHTML serialization
      }
    }
  }
  return return node->to_string(options);
}

Napi::Value XmlNode::Remove(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  node->remove();

  return return info.This();
}

Napi::Value XmlNode::Clone(const Napi::CallbackInfo &info) {
  Napi::HandleScope scope(env);
  XmlNode *node = info.This().Unwrap<XmlNode>();
  assert(node);

  bool recurse = true;

  if (info.Length() == 1 && info[0].IsBoolean())
    recurse = info[0].As<Napi::Boolean>().Value().ToChecked();

  return return node->clone(recurse);
}

Napi::Value XmlNode::New(xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  switch (node->type) {
  case XML_ATTRIBUTE_NODE:
    return scope.Escape(XmlAttribute::New(reinterpret_cast<xmlAttr *>(node)));
  case XML_TEXT_NODE:
    return scope.Escape(XmlText::New(node));
  case XML_PI_NODE:
    return scope.Escape(XmlProcessingInstruction::New(node));
  case XML_COMMENT_NODE:
    return scope.Escape(XmlComment::New(node));

  default:
    // if we don't know how to convert to specific libxmljs wrapper,
    // wrap in an XmlElement.  There should probably be specific
    // wrapper types for text nodes etc., but this is what existing
    // code expects.
    return scope.Escape(XmlElement::New(node));
  }
}

XmlNode::XmlNode(xmlNode *node) : xml_obj(node) {
  xml_obj->_private = this;
  this->ancestor = NULL;

  if ((xml_obj->doc != NULL) && (xml_obj->doc->_private != NULL)) {
    this->doc = xml_obj->doc;
    static_cast<XmlDocument *>(this->doc->_private)->Ref();
  }

  this->ref_wrapped_ancestor();
}

/*
 * Return the (non-document) root, or a wrapped ancestor: whichever is closest
 */
xmlNode *get_wrapped_ancestor_or_root(xmlNode *xml_obj) {
  while ((xml_obj->parent != NULL) &&
         (static_cast<void *>(xml_obj->doc) !=
          static_cast<void *>(xml_obj->parent)) &&
         (xml_obj->parent->_private == NULL)) {
    xml_obj = xml_obj->parent;
  }
  return ((xml_obj->parent != NULL) && (static_cast<void *>(xml_obj->doc) !=
                                        static_cast<void *>(xml_obj->parent)))
             ? xml_obj->parent
             : xml_obj;
}

/*
 * Search linked list for javascript wrapper, ignoring given node.
 */
xmlAttr *get_wrapped_attr_in_list(xmlAttr *xml_obj, void *skip_xml_obj) {
  xmlAttr *wrapped_attr = NULL;
  while (xml_obj != NULL) {
    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != NULL)) {
      wrapped_attr = xml_obj;
      xml_obj = NULL;
    } else {
      xml_obj = xml_obj->next;
    }
  }
  return wrapped_attr;
}

xmlNs *get_wrapped_ns_in_list(xmlNs *xml_obj, void *skip_xml_obj) {
  xmlNs *wrapped_ns = NULL;
  while (xml_obj != NULL) {
    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != NULL)) {
      wrapped_ns = xml_obj;
      xml_obj = NULL;
    } else {
      xml_obj = xml_obj->next;
    }
  }
  return wrapped_ns;
}

xmlNode *get_wrapped_node_in_children(xmlNode *xml_obj, xmlNode *skip_xml_obj);

/*
 * Search document for javascript wrapper, ignoring given node.
 * Based on xmlFreeDoc.
 */
xmlNode *get_wrapped_node_in_document(xmlDoc *xml_obj, xmlNode *skip_xml_obj) {
  xmlNode *wrapped_node = NULL;
  if ((xml_obj->extSubset != NULL) && (xml_obj->extSubset->_private != NULL) &&
      (static_cast<void *>(xml_obj->extSubset) != skip_xml_obj)) {
    wrapped_node = reinterpret_cast<xmlNode *>(xml_obj->extSubset);
  }
  if ((wrapped_node == NULL) && (xml_obj->intSubset != NULL) &&
      (xml_obj->intSubset->_private != NULL) &&
      (static_cast<void *>(xml_obj->intSubset) != skip_xml_obj)) {
    wrapped_node = reinterpret_cast<xmlNode *>(xml_obj->intSubset);
  }
  if ((wrapped_node == NULL) && (xml_obj->children != NULL)) {
    wrapped_node =
        get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }
  if ((wrapped_node == NULL) && (xml_obj->oldNs != NULL)) {
    wrapped_node = reinterpret_cast<xmlNode *>(
        get_wrapped_ns_in_list(xml_obj->oldNs, skip_xml_obj));
  }
  return wrapped_node;
}

/*
 * Search children of node for javascript wrapper, ignoring given node.
 * Based on xmlFreeNodeList.
 */
xmlNode *get_wrapped_node_in_children(xmlNode *xml_obj, xmlNode *skip_xml_obj) {

  xmlNode *wrapped_node = NULL;

  if (xml_obj->type == XML_NAMESPACE_DECL) {
    return reinterpret_cast<xmlNode *>(get_wrapped_ns_in_list(
        reinterpret_cast<xmlNs *>(xml_obj), skip_xml_obj));
  }

  if ((xml_obj->type == XML_DOCUMENT_NODE) ||
#ifdef LIBXML_DOCB_ENABLED
      (xml_obj->type == XML_DOCB_DOCUMENT_NODE) ||
#endif
      (xml_obj->type == XML_HTML_DOCUMENT_NODE)) {
    return get_wrapped_node_in_document(reinterpret_cast<xmlDoc *>(xml_obj),
                                        skip_xml_obj);
  }

  xmlNode *next;
  while (xml_obj != NULL) {
    next = xml_obj->next;

    if ((xml_obj != skip_xml_obj) && (xml_obj->_private != NULL)) {
      wrapped_node = xml_obj;
    } else {

      if ((xml_obj->children != NULL) &&
          (xml_obj->type != XML_ENTITY_REF_NODE)) {
        wrapped_node =
            get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
      }

      if ((wrapped_node == NULL) && ((xml_obj->type == XML_ELEMENT_NODE) ||
                                     (xml_obj->type == XML_XINCLUDE_START) ||
                                     (xml_obj->type == XML_XINCLUDE_END))) {

        if ((wrapped_node == NULL) && (xml_obj->properties != NULL)) {
          wrapped_node = reinterpret_cast<xmlNode *>(
              get_wrapped_attr_in_list(xml_obj->properties, skip_xml_obj));
        }

        if ((wrapped_node == NULL) && (xml_obj->nsDef != NULL)) {
          wrapped_node = reinterpret_cast<xmlNode *>(
              get_wrapped_ns_in_list(xml_obj->nsDef, skip_xml_obj));
        }
      }
    }

    if (wrapped_node != NULL) {
      break;
    }

    xml_obj = next;
  }

  return wrapped_node;
}

/*
 * Search descendants of node to find javascript wrapper,
 * optionally ignoring given node. Based on xmlFreeNode.
 */
xmlNode *get_wrapped_descendant(xmlNode *xml_obj,
                                xmlNode *skip_xml_obj = NULL) {

  xmlNode *wrapped_descendant = NULL;

  if (xml_obj->type == XML_DTD_NODE) {
    return (xml_obj->children == NULL)
               ? NULL
               : get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if (xml_obj->type == XML_NAMESPACE_DECL) {
    return NULL;
  }

  if (xml_obj->type == XML_ATTRIBUTE_NODE) {
    return (xml_obj->children == NULL)
               ? NULL
               : get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if ((xml_obj->children != NULL) && (xml_obj->type != XML_ENTITY_REF_NODE)) {
    wrapped_descendant =
        get_wrapped_node_in_children(xml_obj->children, skip_xml_obj);
  }

  if ((xml_obj->type == XML_ELEMENT_NODE) ||
      (xml_obj->type == XML_XINCLUDE_START) ||
      (xml_obj->type == XML_XINCLUDE_END)) {

    if ((wrapped_descendant == NULL) && (xml_obj->properties != NULL)) {
      wrapped_descendant = reinterpret_cast<xmlNode *>(
          get_wrapped_attr_in_list(xml_obj->properties, skip_xml_obj));
    }

    if ((wrapped_descendant == NULL) && (xml_obj->nsDef != NULL)) {
      wrapped_descendant = reinterpret_cast<xmlNode *>(
          get_wrapped_ns_in_list(xml_obj->nsDef, skip_xml_obj));
    }
  }

  return wrapped_descendant;
}

XmlNode::~XmlNode() {
  if ((this->doc != NULL) && (this->doc->_private != NULL)) {
    static_cast<XmlDocument *>(this->doc->_private)->Unref();
  }
  this->unref_wrapped_ancestor();
  if (xml_obj == NULL)
    return;

  xml_obj->_private = NULL;
  if (xml_obj->parent == NULL) {
    if (get_wrapped_descendant(xml_obj) == NULL) {
      xmlFreeNode(xml_obj);
    }
  } else {
    xmlNode *ancestor = get_wrapped_ancestor_or_root(xml_obj);
    if ((ancestor->_private == NULL) && (ancestor->parent == NULL) &&
        (get_wrapped_descendant(ancestor, xml_obj) == NULL)) {
      xmlFreeNode(ancestor);
    }
  }
}

xmlNode *XmlNode::get_wrapped_ancestor() {
  xmlNode *ancestor = get_wrapped_ancestor_or_root(xml_obj);
  return ((xml_obj == ancestor) || (ancestor->_private == NULL)) ? NULL
                                                                 : ancestor;
}

void XmlNode::ref_wrapped_ancestor() {
  xmlNode *ancestor = this->get_wrapped_ancestor();

  // if our closest wrapped ancestor has changed then we either
  // got removed, added, or a closer ancestor was wrapped
  if (ancestor != this->ancestor) {
    this->unref_wrapped_ancestor();
    this->ancestor = ancestor;
  }

  if (this->ancestor != NULL) {
    XmlNode *node = static_cast<XmlNode *>(this->ancestor->_private);
    node->Ref();
  }
}

void XmlNode::unref_wrapped_ancestor() {
  if ((this->ancestor != NULL) && (this->ancestor->_private != NULL)) {
    (static_cast<XmlNode *>(this->ancestor->_private))->Unref();
  }
  this->ancestor = NULL;
}

Napi::Value XmlNode::get_doc() {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(XmlDocument::New(xml_obj->doc));
}

Napi::Value XmlNode::remove_namespace() {
  xml_obj->ns = NULL;
  return env.Null();
}

Napi::Value XmlNode::get_namespace() {
  Napi::EscapableHandleScope scope(env);
  if (!xml_obj->ns) {
    return scope.Escape(env.Null());
  }

  return scope.Escape(XmlNamespace::New(xml_obj->ns));
}

void XmlNode::set_namespace(xmlNs *ns) {
  xmlSetNs(xml_obj, ns);
  assert(xml_obj->ns);
}

xmlNs *XmlNode::find_namespace(const char *search_str) {
  xmlNs *ns = NULL;

  // Find by prefix first
  ns = xmlSearchNs(xml_obj->doc, xml_obj, (const xmlChar *)search_str);

  // Or find by href
  if (!ns)
    ns = xmlSearchNsByHref(xml_obj->doc, xml_obj, (const xmlChar *)search_str);

  return ns;
}

Napi::Value XmlNode::get_all_namespaces() {
  Napi::EscapableHandleScope scope(env);

  // Iterate through namespaces
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs **nsList = xmlGetNsList(xml_obj->doc, xml_obj);
  if (nsList != NULL) {
    for (int i = 0; nsList[i] != NULL; i++) {
      Napi::Number index = Napi::Number::New(env, i);
      Napi::Object ns = XmlNamespace::New(nsList[i]);
      (namespaces).Set(index, ns);
    }
    xmlFree(nsList);
  }

  return scope.Escape(namespaces);
}

Napi::Value XmlNode::get_local_namespaces() {
  Napi::EscapableHandleScope scope(env);

  // Iterate through local namespaces
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs *nsDef = xml_obj->nsDef;
  for (int i = 0; nsDef; i++, nsDef = nsDef->next) {
    Napi::Number index = Napi::Number::New(env, i);
    Napi::Object ns = XmlNamespace::New(nsDef);
    (namespaces).Set(index, ns);
  }

  return scope.Escape(namespaces);
}

Napi::Value XmlNode::get_parent() {
  Napi::EscapableHandleScope scope(env);

  if (xml_obj->parent) {
    return scope.Escape(XmlElement::New(xml_obj->parent));
  }

  return scope.Escape(XmlDocument::New(xml_obj->doc));
}

Napi::Value XmlNode::get_prev_sibling() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->prev) {
    return scope.Escape(XmlNode::New(xml_obj->prev));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlNode::get_next_sibling() {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->next) {
    return scope.Escape(XmlNode::New(xml_obj->next));
  }

  return scope.Escape(env.Null());
}

Napi::Value XmlNode::get_line_number() {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(Napi::Number::New(env, uint32_t(xmlGetLineNo(xml_obj))));
}

Napi::Value XmlNode::clone(bool recurse) {
  Napi::EscapableHandleScope scope(env);

  xmlNode *new_xml_obj = xmlDocCopyNode(xml_obj, xml_obj->doc, recurse);
  return scope.Escape(XmlNode::New(new_xml_obj));
}

Napi::Value XmlNode::to_string(int options) {
  Napi::EscapableHandleScope scope(env);

  xmlBuffer *buf = xmlBufferCreate();
  const char *enc = "UTF-8";

  xmlSaveCtxt *savectx = xmlSaveToBuffer(buf, enc, options);
  xmlSaveTree(savectx, xml_obj);
  xmlSaveFlush(savectx);

  const xmlChar *xmlstr = xmlBufferContent(buf);

  if (xmlstr) {
    Napi::String str =
        Napi::String::New(env, (char *)xmlstr, xmlBufferLength(buf));
    xmlSaveClose(savectx);

    xmlBufferFree(buf);

    return scope.Escape(str);
  } else {
    xmlSaveClose(savectx);

    xmlBufferFree(buf);

    return scope.Escape(env.Null());
  }
}

void XmlNode::remove() {
  this->unref_wrapped_ancestor();
  xmlUnlinkNode(xml_obj);
}

void XmlNode::add_child(xmlNode *child) { xmlAddChild(xml_obj, child); }

void XmlNode::add_prev_sibling(xmlNode *node) {
  xmlAddPrevSibling(xml_obj, node);
}

void XmlNode::add_next_sibling(xmlNode *node) {
  xmlAddNextSibling(xml_obj, node);
}

xmlNode *XmlNode::import_node(xmlNode *node) {
  if (xml_obj->doc == node->doc) {
    if ((node->parent != NULL) && (node->_private != NULL)) {
      static_cast<XmlNode *>(node->_private)->remove();
    }
    return node;
  } else {
    return xmlDocCopyNode(node, xml_obj->doc, 1);
  }
}

Napi::Value XmlNode::get_type() {
  Napi::EscapableHandleScope scope(env);
  switch (xml_obj->type) {
  case XML_ELEMENT_NODE:
    return scope.Escape(Napi::String::New(env, "element"));
  case XML_ATTRIBUTE_NODE:
    return scope.Escape(Napi::String::New(env, "attribute"));
  case XML_TEXT_NODE:
    return scope.Escape(Napi::String::New(env, "text"));
  case XML_CDATA_SECTION_NODE:
    return scope.Escape(Napi::String::New(env, "cdata"));
  case XML_ENTITY_REF_NODE:
    return scope.Escape(Napi::String::New(env, "entity_ref"));
  case XML_ENTITY_NODE:
    return scope.Escape(Napi::String::New(env, "entity"));
  case XML_PI_NODE:
    return scope.Escape(Napi::String::New(env, "pi"));
  case XML_COMMENT_NODE:
    return scope.Escape(Napi::String::New(env, "comment"));
  case XML_DOCUMENT_NODE:
    return scope.Escape(Napi::String::New(env, "document"));
  case XML_DOCUMENT_TYPE_NODE:
    return scope.Escape(Napi::String::New(env, "document_type"));
  case XML_DOCUMENT_FRAG_NODE:
    return scope.Escape(Napi::String::New(env, "document_frag"));
  case XML_NOTATION_NODE:
    return scope.Escape(Napi::String::New(env, "notation"));
  case XML_HTML_DOCUMENT_NODE:
    return scope.Escape(Napi::String::New(env, "html_document"));
  case XML_DTD_NODE:
    return scope.Escape(Napi::String::New(env, "dtd"));
  case XML_ELEMENT_DECL:
    return scope.Escape(Napi::String::New(env, "element_decl"));
  case XML_ATTRIBUTE_DECL:
    return scope.Escape(Napi::String::New(env, "attribute_decl"));
  case XML_ENTITY_DECL:
    return scope.Escape(Napi::String::New(env, "entity_decl"));
  case XML_NAMESPACE_DECL:
    return scope.Escape(Napi::String::New(env, "namespace_decl"));
  case XML_XINCLUDE_START:
    return scope.Escape(Napi::String::New(env, "xinclude_start"));
  case XML_XINCLUDE_END:
    return scope.Escape(Napi::String::New(env, "xinclude_end"));
  case XML_DOCB_DOCUMENT_NODE:
    return scope.Escape(Napi::String::New(env, "docb_document"));
  }

  return scope.Escape(env.Null());
}

void XmlNode::Initialize(Napi::Object target) {
  Napi::HandleScope scope(env);
  Napi::FunctionReference tmpl = Napi::Napi::FunctionReference::New(env);
  constructor.Reset(tmpl);

  Napi::SetPrototypeMethod(tmpl, "doc", XmlNode::Doc);

  Napi::SetPrototypeMethod(tmpl, "parent", XmlNode::Parent);

  Napi::SetPrototypeMethod(tmpl, "namespace", XmlNode::Namespace);

  Napi::SetPrototypeMethod(tmpl, "namespaces", XmlNode::Namespaces);

  Napi::SetPrototypeMethod(tmpl, "prevSibling", XmlNode::PrevSibling);

  Napi::SetPrototypeMethod(tmpl, "nextSibling", XmlNode::NextSibling);

  Napi::SetPrototypeMethod(tmpl, "line", XmlNode::LineNumber);

  Napi::SetPrototypeMethod(tmpl, "type", XmlNode::Type);

  Napi::SetPrototypeMethod(tmpl, "remove", XmlNode::Remove);

  Napi::SetPrototypeMethod(tmpl, "clone", XmlNode::Clone);

  Napi::SetPrototypeMethod(tmpl, "toString", XmlNode::ToString);

  XmlElement::Initialize(env, target, module);
  XmlText::Initialize(env, target, module);
  XmlComment::Initialize(env, target, module);
  XmlProcessingInstruction::Initialize(env, target, module);
  XmlAttribute::Initialize(env, target, module);
}
} // namespace libxmljs
