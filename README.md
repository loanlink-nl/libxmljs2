# @loanlink-nl/libxmljs2
![this package on npm](https://img.shields.io/npm/v/@loanlink-nl/libxmljs2)

Node-API bindings to the native [libxml2](https://gitlab.gnome.org/GNOME/libxml2) library. 

Works with Node >= 20 (Bun support experimental). 

## Features
- XSD validation
- RelaxNG validation
- Schematron validation
- XPath queries
- SAX parsing
- SAX push parsing
- HTML parsing

If you need anything that's included in libxml2 but not exposed by this module, don't hesitate to [open an issue](https://github.com/loanlink-nl/libxmljs2/issues/new) or to create a PR.

## Usage

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

## Contributing

Contributions are welcome! Follow the following steps to get started:

```sh
# Make sure Bun is installed:
bun --version

# Make sure Python is installed:
python --version

# Initialize the libxml2 submodule (synced from https://gitlab.gnome.org/GNOME/libxml2):
bun run init-submodules

# Install (this will also build the project):
bun install

# Run tests under Node:
bun run test:node

# Run tests under Bun:
bun run test:bun
```

## Changes

Compared to [libxmljs](https://githubcom/libxmljs/libxmljs) and [libxmljs2](https://github.com/marudor/libxmljs2):

- Migrated from NAN to Node-API
- Running + passing test-suite
- Support for Node >= 20
- Experimental Bun support
- Libxml2 2.15.1 (resolving 9 CVEs)
