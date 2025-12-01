import libxml from "../index.js";
import { setupGC } from "./setup.js";

function makeDocument() {
  const body =
    "<?xml version='1.0' encoding='UTF-8'?>\n" +
    '<root><outer><middle><inner><left/><center/><right/></inner></middle></outer></root>';

  return libxml.parseXml(body);
}

describe('ref integrity', () => {
  it('simple gc', async() => {
    await new Promise((done) => {
      const doc = new libxml.Document();

      doc.node('root');

      global.gc(true);
      expect(doc).toBeTruthy();

      global.gc(true);
      setTimeout(() => {
        expect(doc.root()).toBeTruthy();
        done();
      }, 1);
    });
  });

  it('gc', () => {
    const doc = new libxml.Document();

    doc.node('root').node('child').node('grandchild').parent().node('child2');
    global.gc(true);
    expect(doc).toBeTruthy();
    global.gc(true);
    expect(doc.root()).toBeTruthy();
    global.gc(true);
    expect('child').toBe(doc.root().childNodes()[0].name());
  });

  it('references', () => {
    const nodes = libxml
      .parseXml('<root> <child> <grandchildren/> </child> <child2/> </root>')
      .childNodes();

    global.gc(true);

    expect(nodes[0].doc()).toBeTruthy();
    expect(nodes[1].name()).toBe('child');
  });

  // test that double-freeing XmlNode's doesn't cause a segfault
  it('double_free', () => {
    let children = null;

    // stick this portion of code into a self-executing function so
    // its internal variables can be garbage collected
    (function internal() {
      const html = '<html><body><div><span></span></div></body></html>';
      const doc = libxml.parseHtml(html);

      for (const tag of doc.find('//div')) {
        // provide a reference to childNodes so they are exposed as XmlNodes
        // and therefore subject to V8's garbage collection
        children = tag.childNodes();
        tag.remove();
      }
    })();

    global.gc(true);
    expect(children[0].attrs()).toBeTruthy();
  });

  // eslint-disable-next-line jest/expect-expect
  it('freed_namespace_unwrappable', () => {
    const doc = libxml.parseXml(
      "<?xml version='1.0' encoding='UTF-8'?><root></root>"
    );
    let el = new libxml.Element(doc, 'foo');
    // eslint-disable-next-line no-unused-vars
    let ns = el.namespace('bar', null);

    el = null;
    global.gc(true);
    ns = null;
    global.gc(true);
  });

  it.skip('unlinked_tree_persistence_parent_proxied_first', async () => {
    const { traceGC, awaitGC } = setupGC();

    const doc = makeDocument();
    let parent_node = doc.get('//middle');
    traceGC(parent_node, 'parent_node');

    const child_node = doc.get('//inner');

    parent_node.remove();
    parent_node = null;
    await awaitGC('parent_node');

    expect(child_node.name()).toBe('inner'); // works with >= v0.14.3
  });

  it.skip('unlinked_tree_proxied_leaf_persistent_ancestor_first', async () => {
    const { traceGC, awaitGC } = setupGC();
    const doc = makeDocument();
    let ancestor = doc.get('//middle');
    traceGC(ancestor, 'ancestor');

    const leaf = doc.get('//center');

    ancestor.remove();
    ancestor = null;
    await awaitGC('ancestor');

    expect(leaf.name()).toBe('center'); // fails with v0.14.3, v0.15
  });

  it('unlinked_tree_proxied_leaf_persistent_descendant_first', async () => {
    const { traceGC, awaitGC } = setupGC();
    const doc = makeDocument();
    const leaf = doc.get('//center');
    let ancestor = doc.get('//middle');
    traceGC(ancestor, 'ancestor');

    ancestor.remove(); // make check here?
    ancestor = null;
    await awaitGC('ancestor');

    expect(leaf.name()).toBe('center');
  });

  it('unlinked_tree_persistence_child_proxied_first', async () => {
    const { traceGC, awaitGC } = setupGC();
    const doc = makeDocument();
    const child_node = doc.get('//inner');
    let parent_node = doc.get('//middle');
    traceGC(parent_node, 'parent_node');

    parent_node.remove();
    parent_node = null;
    await awaitGC('parent_node');

    expect(child_node.name()).toBe('inner'); // fails with v0.14.3, v0.15
  });

  (typeof Bun !== 'undefined' ? it.skip : it)('unlinked_tree_leaf_persistence_with_proxied_ancestor', async () => {
    const { traceGC, awaitGC } = setupGC();

    const doc = makeDocument();
    const proxied_ancestor = doc.get('//inner');
    let leaf = doc.get('//center');
    traceGC(leaf, 'leaf');

    doc.get('//middle').remove();

    leaf = null;
    await awaitGC('leaf');

    leaf = proxied_ancestor.get('.//center');
    expect(leaf.name()).toBe('center');
  });

  (typeof Bun !== 'undefined' ? it.skip : it)('unlinked_tree_leaf_persistence_with_peer_proxy', async () => {
    const { traceGC, awaitGC } = setupGC();
      const doc = makeDocument();
      let leaf = doc.get('//left');
      traceGC(leaf, 'leaf');

      const peer = doc.get('//right');

      doc.get('//middle').remove();
      leaf = null;
      await awaitGC('leaf');

      leaf = peer.parent().get('./left');
      expect(leaf.name()).toBe('left');
  });


  it('set_text_clobbering_children', () => {
    const doc = libxml.parseXml(
      '<root><child><inner>old</inner></child></root>'
    );
    const child = doc.get('//child');
    const inner = doc.get('//inner');

    child.text('new');

    expect(inner.parent()).toBe(doc);
    expect(inner.text()).toBe('old');
  });

  it("doesn't segfault", async () => {
    const doc = libxml.parseXml('<doc />');
    doc.get('//doc').remove();
    global.gc(true);

    const doc2 = libxml.parseXml('<doc2 />');
    doc2.get('//doc2')

    global.gc(true);
  });

});
