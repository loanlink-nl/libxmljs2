import { $ } from "bun";

const platform = process.platform; // 'darwin', 'linux', 'win32'

if (platform === "darwin" || platform === "linux") {
  await $`cd vendor/libxml2 && ./autogen.sh --with-schematron && cd ../..`;
} else if (platform === "win32") {
  await $`cd vendor/libxml2 && cmake . --with-schematron && cd ../..`;
} 
