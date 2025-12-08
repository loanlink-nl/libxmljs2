# @loanlink-nl/libxmljs2

N-API bindings to the native [libxml2](https://gitlab.gnome.org/GNOME/libxml2) library. 

Works with Node (Bun support experimental). 

Features:
- XSD validation
- RelaxNG validation
- Schematron validation
- XPath queries
- SAX parsing
- SAX push parsing
- HTML parsing
- HTML fragment parsing

Usage:

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

const gchild = xmlDoc.get('//grandchild');

console.log(gchild.text()); 
// "grandchild content"

const children = xmlDoc.root().childNodes();
const child = children[0];

console.log(child.attr('foo').value()); 
// "bar"
```

If you need anything that's included in libxml2 but not exposed by this module, don't hesitate to [open an issue](https://github.com/loanlink-nl/libxmljs2/issues/new) or to create a PR.
