const libxml = require('../index');

global.gc ??= Bun.gc;
if (!global.gc) {
  throw new Error('must run with --expose_gc for memory management tests');
}

function makeDocument() {
  const body =
    "<?xml version='1.0' encoding='UTF-8'?>\n" +
    '<root><outer><middle><inner><center/></inner></middle></outer></root>';

  return libxml.parseXml(body);
}


describe('memory management', () => {
  beforeEach(() => {
    global.gc(true);
  });

  afterEach(() => {
    global.gc(true);
  });

  it('inaccessible document freed', async () => {
    const xml_memory_before_document = libxml.memoryUsage();

    await new Promise((done) => {
      for (let i = 0; i < 100; i += 1) {
        makeDocument();
      }

      global.gc(true);

      setTimeout(() => {
        done();
      }, 1);
    });

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  });

  it("calling root doesn't keep the document alive", async () => {
    const xml_memory_before_document = libxml.memoryUsage();

    await new Promise((done) => {
      let doc1 = new libxml.Document();
      let doc2 = new libxml.Document("2.0");

      doc1.type();

      doc1 = null;
      doc2 = null;

      global.gc(true);

      process.nextTick(() => {
        global.gc(true);

        setTimeout(() => {
          done();
        }, 1);
      });
    });

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  });

  it('inaccessible document freed when node freed', async () => {
    const xml_memory_before_document = libxml.memoryUsage();

    await new Promise((done) => {
      let nodes = [];
      for (let i = 0; i < 100; i += 1) {
        nodes.push(makeDocument().get('//center'));
      }
      nodes = null;

      global.gc(true);

      setTimeout(() => {
        global.gc(true);

        setTimeout(() => {
          done();
        }, 1);
      }, 1);
    })

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  }, { retry: 10 });

  it('inaccessible document freed after middle nodes proxies', async () => {
    const xml_memory_before_document = libxml.memoryUsage();

    await new Promise((done) => {
      let doc = makeDocument();
      // eslint-disable-next-line no-unused-vars
      let middle = doc.get('//middle');
      let inner = doc.get('//inner');

      inner.remove(); // v0.14.3, v0.15: proxy ref'd parent but can't unref when destroyed
      doc = middle = inner = null;

      global.gc(true);

      setTimeout(() => {
        global.gc(true);

        setTimeout(() => {
          done();
        }, 1);
      }, 1);
    });

    expect(libxml.memoryUsage() <= xml_memory_before_document).toBeTruthy();
  }, { retry: 10 });

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
    let xmlMemBefore;

    await new Promise((done) => {
      const doc = makeDocument();
      const el = doc.get('//center');

      el.namespace('bar', null);
      xmlMemBefore = libxml.memoryUsage();

      for (let i; i < 1000; i += 1) {
        el.namespaces();
      }

      global.gc(true);
      setTimeout(() => {
        done();
      }, 1);
    });

    expect(libxml.memoryUsage() <= xmlMemBefore).toBeTruthy();
  });
});
