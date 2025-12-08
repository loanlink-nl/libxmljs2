// js acts as a wrapper to the c++ bindings
// prefer to do error handling and other abstrctions in the
// js layer and only go to c++ when we need to hit libxml
import bindings from "./lib/bindings.js";

// document parsing for backwards compat
import Document from "./lib/document.js";
import Element from "./lib/element.js";
import { SaxParser, SaxPushParser } from "./lib/sax_parser.js";

export default {
  parseXml: Document.fromXml,
  parseHtml: Document.fromHtml,
  parseHtmlFragment: Document.fromHtmlFragment,
  version: "0.0.3",
  libxml_version: bindings.libxml_version,
  libxml_parser_version: bindings.libxml_parser_version,
  libxml_debug_enabled: bindings.libxml_debug_enabled,
  features: bindings.features,
  Comment: bindings.Comment,
  Document,
  Element,
  ProcessingInstruction: bindings.ProcessingInstruction,
  Text: bindings.Text,
  SaxParser,
  SaxPushParser,
  memoryUsage: bindings.xmlMemUsed,
  nodeCount: bindings.xmlNodeCount,
  TextWriter: bindings.TextWriter,
  parseXmlString: Document.fromXml,
  parseHtmlString: Document.fromHtml,
};

