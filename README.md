# @loanlink-nl/libxmljs2

N-API bindings to the native [libxml2](https://gitlab.gnome.org/GNOME/libxml2) library. 

Works with Node (Bun experimental). 

Features:
- XSD validation
- RelaxNG validation
- Schematron validation
- XPath queries
- SAX parsing
- SAX push parsing
- HTML parsing
- HTML fragment parsing

```javascript
import * as libxmljs from '@loanlink-nl/libxmljs2';

const xml =
  '<?xml version="1.0" encoding="UTF-8"?>' +
  '<root>' +
  '<child foo="bar">' +
  '<grandchild baz="fizbuzz">grandchild content</grandchild>' +
  '</child>' +
  '<sibling>with content!</sibling>' +
  '</root>';

const xmlDoc = libxmljs.parseXml(xml);

// xpath queries
const gchild = xmlDoc.get('//grandchild');

console.log(gchild.text()); // prints "grandchild content"

const children = xmlDoc.root().childNodes();
const child = children[0];

console.log(child.attr('foo').value()); // prints "bar"
```
