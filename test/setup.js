global.gc ??= (typeof Bun !== 'undefined' ? Bun.gc : undefined);
if (!global.gc) {
  throw new Error('must run with --expose-gc for memory management tests');
}

export function setupGC() {
  const registry = new FinalizationRegistry(message => {
    events.add(message)
  });

  const events = new Set()

  function traceGC(obj, message) {
    registry.register(obj, message)
  }

  async function awaitGC(message) {
    return new Promise(resolve => {
      global.gc()

      const timer = setInterval(() => {
        if (events.has(message)) {
          events.delete(message)
          clearInterval(timer)
          resolve()
        }
      })
    })
  }

  return { traceGC, awaitGC };
}

