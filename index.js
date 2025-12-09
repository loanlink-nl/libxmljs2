// js acts as a wrapper to the c++ bindings
// prefer to do error handling and other abstrctions in the
// js layer and only go to c++ when we need to hit libxml
import bindings from "./lib/bindings.js";

import Document from "./lib/document.js";
export { SaxParser, SaxPushParser } from "./lib/sax_parser.js";

export { default as Document } from "./lib/document.js";
export { default as Element } from "./lib/element.js";

export const parseXml = Document.fromXml;
export const parseHtml = Document.fromHtml;
export const parseHtmlFragment = Document.fromHtmlFragment;
export const parseXmlString = Document.fromXml;
export const parseHtmlString = Document.fromHtml;
export const version = "0.0.7";
export const libxml_version = bindings.libxml_version;
export const libxml_parser_version = bindings.libxml_parser_version;
export const libxml_debug_enabled = bindings.libxml_debug_enabled;
export const features = bindings.features;
export const Comment = bindings.Comment;
export const ProcessingInstruction = bindings.ProcessingInstruction;
export const Text = bindings.Text;
export const memoryUsage = bindings.xmlMemUsed;
export const nodeCount = bindings.xmlNodeCount;
export const TextWriter = bindings.TextWriter;

