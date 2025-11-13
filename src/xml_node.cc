// Copyright 2009, Squish Tech, LLC.

#include <libxml/xmlsave.h>

#include "xml_attribute.h"
#include "xml_comment.h"
#include "xml_document.h"
#include "xml_element.h"
#include "xml_namespace.h"
#include "xml_node.h"
#include "xml_pi.h"
#include "xml_text.h"

namespace libxmljs {

template <class T> Napi::FunctionReference XmlNode<T>::constructor;

template <class T>
XmlNode<T>::XmlNode(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<T>(info) {
  // Napi::Env env = info.Env();

  // If called with no arguments, the derived class constructor will set xml_obj
  // if (info.Length() == 0) {
  this->xml_obj = NULL;
  this->ancestor = NULL;
  this->doc = NULL;
  //   return;
  // }

  // if (!info[0].IsExternal()) {
  //   return;
  // }
  //
  // auto external = info[0].As<Napi::External<xmlNode>>();
  // xmlNode *data = external.Data();
  //
  // this->xml_obj = data;
  // this->xml_obj->_private = this;
  // this->ancestor = NULL;
  //
  // if ((xml_obj->doc != NULL) && (xml_obj->doc->_private != NULL)) {
  //   this->doc = xml_obj->doc;
  //
  //   XmlDocument *doc = static_cast<XmlDocument *>(this->doc->_private);
  //   printf("ref doc node\n");
  //   fflush(stdout);
  //   doc->Ref();
  // }
  //
  // this->Value().Set("_xmlNode",
  //                   Napi::External<xmlNode>::New(env, this->xml_obj));
  // this->ref_wrapped_ancestor();
}

template <class T> Napi::Value XmlNode<T>::Doc(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_doc(env));
}

template <class T>
Napi::Value XmlNode<T>::Namespace(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  // #namespace() Get the node's namespace
  if (info.Length() == 0) {
    return scope.Escape(this->get_namespace(env));
  }

  if (info[0].IsNull())
    return scope.Escape(this->remove_namespace(env));

  XmlNamespace *ns = NULL;

  // #namespace(ns) libxml.Namespace object was provided
  // TODO(sprsquish): check that it was actually given a namespace obj
  if (info[0].IsObject())
    ns = Napi::ObjectWrap<XmlNamespace>::Unwrap(info[0].As<Napi::Object>());

  // #namespace(href) or #namespace(prefix, href)
  // if the namespace has already been defined on the node, just set it
  if (info[0].IsString()) {
    std::string ns_to_find = info[0].As<Napi::String>().Utf8Value();
    xmlNs *found_ns = this->find_namespace(ns_to_find.c_str());
    if (found_ns) {
      // maybe build
      Napi::Object existing =
          XmlNamespace::NewInstance(env, found_ns).ToObject();
      ns = Napi::ObjectWrap<XmlNamespace>::Unwrap(existing);
    }
  }

  // Namespace does not seem to exist, so create it.
  if (!ns) {
    const unsigned int argc = 3;
    std::vector<napi_value> argv(argc);
    argv[0] = info.This();

    if (info.Length() == 1) {
      argv[1] = env.Null();
      argv[2] = info[0];
    } else {
      argv[1] = info[0];
      argv[2] = info[1];
    }

    Napi::Function define_namespace = XmlNamespace::constructor.Value();

    // will create a new namespace attached to this node
    // since we keep the document around, the namespace, like the node, won't be
    // garbage collected
    Napi::Object new_ns = define_namespace.New(argv);
    ns = Napi::ObjectWrap<XmlNamespace>::Unwrap(new_ns);
  }

  this->set_namespace(ns->xml_obj);
  return scope.Escape(info.This());
}

template <class T>
Napi::Value XmlNode<T>::Namespaces(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  // ignore everything but a literal true; different from IsFalse
  if ((info.Length() == 0) || !info[0].IsBoolean() ||
      !info[0].As<Napi::Boolean>().Value()) {
    return scope.Escape(this->get_all_namespaces(env));
  }

  return scope.Escape(this->get_local_namespaces(env));
}

template <class T>
Napi::Value XmlNode<T>::Parent(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_parent(env));
}

template <class T>
Napi::Value XmlNode<T>::PrevSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_prev_sibling(env));
}

template <class T>
Napi::Value XmlNode<T>::NextSibling(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_next_sibling(env));
}

template <class T>
Napi::Value XmlNode<T>::LineNumber(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_line_number(env));
}

template <class T>
Napi::Value XmlNode<T>::Type(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(this->get_type(env));
}

template <class T>
Napi::Value XmlNode<T>::ToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  int options = 0;

  if (info.Length() > 0) {
    if (info[0].IsBoolean()) {
      if (info[0].As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_FORMAT;
      }
    } else if (info[0].IsObject()) {
      Napi::Object obj = info[0].As<Napi::Object>();

      // drop the xml declaration
      if (obj.Has("declaration") && obj.Get("declaration").IsBoolean() &&
          !obj.Get("declaration").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_NO_DECL;
      }

      // format save output
      if (obj.Has("format") && obj.Get("format").IsBoolean() &&
          obj.Get("format").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_FORMAT;
      }

      // no empty tags (only works with XML) ex: <title></title> becomes
      // <title/>
      if (obj.Has("selfCloseEmpty") && obj.Get("selfCloseEmpty").IsBoolean() &&
          !obj.Get("selfCloseEmpty").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_NO_EMPTY;
      }

      // format with non-significant whitespace
      if (obj.Has("whitespace") && obj.Get("whitespace").IsBoolean() &&
          obj.Get("whitespace").As<Napi::Boolean>().Value()) {
        options |= XML_SAVE_WSNONSIG;
      }

      if (obj.Has("type") && obj.Get("type").IsString()) {
        std::string type = obj.Get("type").As<Napi::String>().Utf8Value();
        if (type == "XML" || type == "xml") {
          options |= XML_SAVE_AS_XML; // force XML serialization on HTML doc
        } else if (type == "HTML" || type == "html") {
          options |= XML_SAVE_AS_HTML; // force HTML serialization on XML doc
          // if the document is XML and we want formatted HTML output
          // we must use the XHTML serializer because the default HTML
          // serializer only formats node->type = HTML_NODE and not XML_NODEs
          if ((options & XML_SAVE_FORMAT) &&
              (options & XML_SAVE_XHTML) == false) {
            options |= XML_SAVE_XHTML;
          }
        } else if (type == "XHTML" || type == "xhtml") {
          options |= XML_SAVE_XHTML; // force XHTML serialization
        }
      }
    }
  }

  return scope.Escape(this->to_string(env, options));
}

template <class T>
Napi::Value XmlNode<T>::Remove(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  this->remove();

  return scope.Escape(info.This());
}

template <class T>
Napi::Value XmlNode<T>::Clone(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  bool recurse = true;

  if (info.Length() == 1 && info[0].IsBoolean())
    recurse = info[0].ToBoolean().Value();

  return scope.Escape(this->clone(env, recurse));
}

template <class T>
Napi::Value XmlNode<T>::NewInstance(Napi::Env env, xmlNode *node) {
  Napi::EscapableHandleScope scope(env);
  switch (node->type) {
  case XML_ATTRIBUTE_NODE:
    return scope.Escape(
        XmlAttribute::NewInstance(env, reinterpret_cast<xmlAttr *>(node)));
  case XML_TEXT_NODE:
    return scope.Escape(XmlText::NewInstance(env, node));
  case XML_PI_NODE:
    return scope.Escape(XmlProcessingInstruction::NewInstance(env, node));
  case XML_COMMENT_NODE:
    return scope.Escape(XmlComment::NewInstance(env, node));

  default:
    // if we don't know how to convert to specific libxmljs wrapper,
    // wrap in an XmlElement.  There should probably be specific
    // wrapper types for text nodes etc., but this is what existing
    // code expects.
    return scope.Escape(XmlElement::NewInstance(env, node));
  }
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

template <class T> XmlNode<T>::~XmlNode() {
  if ((this->doc != NULL) && (this->doc->_private != NULL)) {
    static_cast<XmlDocument *>(this->doc->_private)->Unref();
  }

  this->unref_wrapped_ancestor();
  if (this->xml_obj == NULL) {
    return;
  }

  this->xml_obj->_private = NULL;
  if (this->xml_obj->parent == NULL) {
    if (get_wrapped_descendant(this->xml_obj) == NULL) {
      xmlFreeNode(this->xml_obj);
    }
  } else {
    xmlNode *ancestor = get_wrapped_ancestor_or_root(this->xml_obj);
    if ((ancestor->_private == NULL) && (ancestor->parent == NULL) &&
        (get_wrapped_descendant(ancestor, this->xml_obj) == NULL)) {
      xmlFreeNode(ancestor);
    }
  }
}

template <class T> xmlNode *XmlNode<T>::get_wrapped_ancestor() {
  xmlNode *ancestor = get_wrapped_ancestor_or_root(this->xml_obj);
  return ((this->xml_obj == ancestor) || (ancestor->_private == NULL))
             ? NULL
             : ancestor;
}

template <class T> void XmlNode<T>::ref_wrapped_ancestor() {
  xmlNode *ancestor = this->get_wrapped_ancestor();

  // if our closest wrapped ancestor has changed then we either
  // got removed, added, or a closer ancestor was wrapped
  if (ancestor != this->ancestor) {
    this->unref_wrapped_ancestor();
    this->ancestor = ancestor;
  }

  if (this->ancestor != NULL) {
    this->Value().Set("_ancestor", static_cast<XmlNode *>(this->ancestor->_private)->Value());
  }
}

template <class T> void XmlNode<T>::unref_wrapped_ancestor() {
  if ((this->ancestor != NULL) && (this->ancestor->_private != NULL)) {
    Napi::Value val = this->Value();
    if (val.IsObject()) {
      val.As<Napi::Object>().Delete("_ancestor");
    }
  }
  this->ancestor = NULL;
}

template <class T> Napi::Value XmlNode<T>::get_doc(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(XmlDocument::NewInstance(env, this->xml_obj->doc));
}

template <class T> Napi::Value XmlNode<T>::remove_namespace(Napi::Env env) {
  this->xml_obj->ns = NULL;
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(env.Null());
}

template <class T> Napi::Value XmlNode<T>::get_namespace(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (!this->xml_obj->ns) {
    return scope.Escape(env.Null());
  }

  return scope.Escape(XmlNamespace::NewInstance(env, this->xml_obj->ns));
}

template <class T> void XmlNode<T>::set_namespace(xmlNs *ns) {
  xmlSetNs(this->xml_obj, ns);
  assert(this->xml_obj->ns);
}

template <class T> xmlNs *XmlNode<T>::find_namespace(const char *search_str) {
  xmlNs *ns = NULL;

  // Find by prefix first
  ns = xmlSearchNs(this->xml_obj->doc, xml_obj, (const xmlChar *)search_str);

  // Or find by href
  if (!ns)
    ns = xmlSearchNsByHref(this->xml_obj->doc, xml_obj,
                           (const xmlChar *)search_str);

  return ns;
}

template <class T> Napi::Value XmlNode<T>::get_all_namespaces(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  // Iterate through namespaces
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs **nsList = xmlGetNsList(this->xml_obj->doc, this->xml_obj);
  if (nsList != NULL) {
    for (int i = 0; nsList[i] != NULL; i++) {
      Napi::Object ns = XmlNamespace::NewInstance(env, nsList[i]).ToObject();
      namespaces.Set(i, ns);
    }
    xmlFree(nsList);
  }

  return scope.Escape(namespaces);
}

template <class T> Napi::Value XmlNode<T>::get_local_namespaces(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  // Iterate through local namespaces
  Napi::Array namespaces = Napi::Array::New(env);
  xmlNs *nsDef = xml_obj->nsDef;
  for (int i = 0; nsDef; i++, nsDef = nsDef->next) {
    Napi::Object ns = XmlNamespace::NewInstance(env, nsDef).ToObject();
    namespaces.Set(i, ns);
  }

  return scope.Escape(namespaces);
}

template <class T> Napi::Value XmlNode<T>::get_parent(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->parent) {
    return scope.Escape(XmlElement::NewInstance(env, xml_obj->parent));
  }

  return scope.Escape(XmlDocument::NewInstance(env, xml_obj->doc));
}

template <class T> Napi::Value XmlNode<T>::get_prev_sibling(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->prev) {
    return scope.Escape(XmlNode::NewInstance(env, xml_obj->prev));
  }

  return scope.Escape(env.Null());
}

template <class T> Napi::Value XmlNode<T>::get_next_sibling(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  if (xml_obj->next) {
    return scope.Escape(XmlNode::NewInstance(env, xml_obj->next));
  }

  return scope.Escape(env.Null());
}

template <class T> Napi::Value XmlNode<T>::get_line_number(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  return scope.Escape(
      Napi::Number::New(env, static_cast<uint32_t>(xmlGetLineNo(xml_obj))));
}

template <class T> Napi::Value XmlNode<T>::clone(Napi::Env env, bool recurse) {
  Napi::EscapableHandleScope scope(env);
  xmlNode *new_xml_obj = xmlDocCopyNode(xml_obj, xml_obj->doc, recurse);
  return scope.Escape(XmlNode::NewInstance(env, new_xml_obj));
}

template <class T>
Napi::Value XmlNode<T>::to_string(Napi::Env env, int options) {
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

template <class T> void XmlNode<T>::remove() {
  this->unref_wrapped_ancestor();
  // Clear the document reference when node is removed from tree
  Napi::Value val = this->Value();
  if (val.IsObject()) {
    val.As<Napi::Object>().Delete("document");
  }
  xmlUnlinkNode(xml_obj);
}

template <class T> void XmlNode<T>::add_child(xmlNode *child) {
  xmlAddChild(xml_obj, child);
}

template <class T> void XmlNode<T>::add_prev_sibling(xmlNode *node) {
  xmlAddPrevSibling(xml_obj, node);
}

template <class T> void XmlNode<T>::add_next_sibling(xmlNode *node) {
  xmlAddNextSibling(xml_obj, node);
}

template <class T> xmlNode *XmlNode<T>::import_node(xmlNode *node) {
  if (xml_obj->doc == node->doc) {
    if ((node->parent != NULL) && (node->_private != NULL)) {
      static_cast<XmlNode *>(node->_private)->remove();
    }
    return node;
  } else {
    return xmlDocCopyNode(node, xml_obj->doc, 1);
  }
}

template <class T> Napi::Value XmlNode<T>::get_type(Napi::Env env) {
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

  return env.Null();
}

template <class T>
Napi::Function XmlNode<T>::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = Napi::ObjectWrap<T>::DefineClass(
      env, "XmlNode",
      {
          Napi::ObjectWrap<T>::InstanceMethod("doc", &XmlNode::Doc),
          Napi::ObjectWrap<T>::InstanceMethod("parent", &XmlNode::Parent),
          Napi::ObjectWrap<T>::InstanceMethod("namespace", &XmlNode::Namespace),
          Napi::ObjectWrap<T>::InstanceMethod("namespaces",
                                              &XmlNode::Namespaces),
          Napi::ObjectWrap<T>::InstanceMethod("prevSibling",
                                              &XmlNode::PrevSibling),
          Napi::ObjectWrap<T>::InstanceMethod("nextSibling",
                                              &XmlNode::NextSibling),
          Napi::ObjectWrap<T>::InstanceMethod("line", &XmlNode::LineNumber),
          Napi::ObjectWrap<T>::InstanceMethod("type", &XmlNode::Type),
          Napi::ObjectWrap<T>::InstanceMethod("toString", &XmlNode::ToString),
          Napi::ObjectWrap<T>::InstanceMethod("remove", &XmlNode::Remove),
          Napi::ObjectWrap<T>::InstanceMethod("clone", &XmlNode::Clone),
      });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  return func;
}

// The magic happens here
Napi::Value SetupXmlNodeInheritance(Napi::Env env, Napi::Object exports) {
  Napi::Function XmlNode = XmlNodeInstance::Init(env, exports);
  Napi::Function XmlElement = XmlElement::Init(env, exports);
  Napi::Function XmlText = XmlText::Init(env, exports);
  Napi::Function XmlComment = XmlComment::Init(env, exports);
  Napi::Function XmlProcessingInstruction =
      XmlProcessingInstruction::Init(env, exports);
  Napi::Function XmlAttribute = XmlAttribute::Init(env, exports);

  exports.Set("XmlNode", XmlNode);
  exports.Set("Element", XmlElement);
  exports.Set("Text", XmlText);
  exports.Set("Comment", XmlComment);
  exports.Set("ProcessingInstruction", XmlProcessingInstruction);
  exports.Set("Attribute", XmlAttribute);

  Napi::Function setProto = env.Global()
                                .Get("Object")
                                .ToObject()
                                .Get("setPrototypeOf")
                                .As<Napi::Function>();
  setProto.Call({XmlElement, XmlNode});
  setProto.Call({XmlElement.Get("prototype"), XmlNode.Get("prototype")});

  setProto.Call({XmlText, XmlNode});
  setProto.Call({XmlText.Get("prototype"), XmlNode.Get("prototype")});

  setProto.Call({XmlComment, XmlNode});
  setProto.Call({XmlComment.Get("prototype"), XmlNode.Get("prototype")});

  setProto.Call({XmlProcessingInstruction, XmlNode});
  setProto.Call(
      {XmlProcessingInstruction.Get("prototype"), XmlNode.Get("prototype")});

  setProto.Call({XmlAttribute, XmlNode});
  setProto.Call({XmlAttribute.Get("prototype"), XmlNode.Get("prototype")});

  return exports;
}

XmlNodeInstance::~XmlNodeInstance() {}

// Explicit template instantiations for all XmlNode derivatives
template class XmlNode<XmlElement>;
template class XmlNode<XmlAttribute>;
template class XmlNode<XmlText>;
template class XmlNode<XmlComment>;
template class XmlNode<XmlProcessingInstruction>;
template class XmlNode<XmlNodeInstance>;

} // namespace libxmljs
