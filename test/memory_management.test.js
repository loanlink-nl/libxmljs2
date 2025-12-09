import * as libxml from "../index.js";
import { setupGC } from "./setup.js";

function makeDocument() {
  const body =
    "<?xml version='1.0' encoding='UTF-8'?>\n" +
    '<root><outer><middle><inner><center/></inner></middle></outer></root>';

  return libxml.parseXml(body);
}


describe('memory management', () => {
  it('inaccessible document freed', async () => {
    const { traceGC, awaitGC } = setupGC();

    const xml_memory_before_document = libxml.memoryUsage();

    for (let i = 0; i < 100; i += 1) {
      traceGC(makeDocument(), `doc-${i}`);
    }

    await awaitGC('doc-0');
    global.gc(true);
    await new Promise(resolve => setTimeout(resolve, 1));

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  });

  it("calling root doesn't keep the document alive", async () => {
    const { traceGC, awaitGC } = setupGC();

    const xml_memory_before_document = libxml.memoryUsage();

    let doc1 = new libxml.Document();
    let doc2 = new libxml.Document("2.0");
    traceGC(doc1, 'doc1');
    traceGC(doc2, 'doc2');

    doc1.type();

    doc1 = null;
    doc2 = null;

    await awaitGC('doc1');
    await awaitGC('doc2');
    global.gc(true);
    await new Promise(resolve => setTimeout(resolve, 1));

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  }, 10_000);

  (typeof Bun !== 'undefined' ? it.skip : it)('inaccessible document freed when node freed', async () => {
    const { traceGC, awaitGC } = setupGC();
    const xml_memory_before_document = libxml.memoryUsage();

    let nodes = [];
    traceGC(nodes, 'nodes');

    for (let i = 0; i < 100; i += 1) {
      let doc = makeDocument();
      traceGC(doc, `doc-${i}`);
      nodes.push(doc.get('//center'));
      doc = null;
    }

    nodes = null;

    await awaitGC('nodes');
    await awaitGC('doc-99');

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  });

  (typeof Bun !== 'undefined' ? it.skip : it)('inaccessible document freed after middle node proxies', async () => {
    const { traceGC, awaitGC } = setupGC();
    const xml_memory_before_document = libxml.memoryUsage();

    let doc = makeDocument();
    // eslint-disable-next-line no-unused-vars
    let middle = doc.get('//middle');
    let inner = doc.get('//inner');

    traceGC(doc, 'doc');
    traceGC(middle, 'middle');
    traceGC(inner, 'inner');

    inner.remove(); // v0.14.3, v0.15: proxy ref'd parent but can't unref when destroyed
    doc = middle = inner = null;

    await awaitGC('doc');
    await awaitGC('middle');
    await awaitGC('inner');

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  });

  it('inaccessible tree freed', async () => {
    let xml_memory_after_document;

    await new Promise((done) => {
      const doc = makeDocument();
      xml_memory_after_document = libxml.memoryUsage();

      doc.get('//middle').remove();
      global.gc(true);
      setTimeout(() => {
        done();
      }, 1);
    });

    expect(libxml.memoryUsage() <= xml_memory_after_document).toBeTruthy();
  });

  it('namespace list freed', async () => {
    const { traceGC, awaitGC } = setupGC();
    let xmlMemBefore;

    const doc = makeDocument();
    const el = doc.get('//center');
    el.namespace('bar', null);

    xmlMemBefore = libxml.memoryUsage();

    for (let i = 0; i < 1000; i += 1) {
      traceGC(el.namespaces(), `namespaces-${i}`);
    }

    await awaitGC(`namespaces-0`);

    expect(libxml.memoryUsage() <= xmlMemBefore).toBeTruthy();
  }, 10_000);
});
