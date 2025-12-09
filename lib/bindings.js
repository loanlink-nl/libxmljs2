import b from "node-gyp-build";
import path from "node:path";
export default b(path.join(import.meta.dirname, '..'));

