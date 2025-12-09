import * as libxml from "../index.js";

const doc = new libxml.Document();
const elem = doc.node('name1');
let newChild = new libxml.Element(doc, 'new-child');
elem.addChild(newChild);

elem.node('child1');
const child2 = elem.node('child2', 'second');
newChild = new libxml.Element(doc, 'new-child');
const name2 = elem.node('name2');
name2.addChild(newChild);
child2.cdata('<h1>cdata test</h1>').cdata('<p>It\'s worked</p>').cdata('<hr/>All done');

console.log('Document with CDATA: %s', doc.toString());
