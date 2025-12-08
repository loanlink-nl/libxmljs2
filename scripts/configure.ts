import { $ } from "bun";

const platform = process.platform; // 'darwin', 'linux', 'win32'

if (platform === "darwin" || platform === "linux") {
  await $`cd vendor/libxml2 && ./autogen.sh --with-schematron && cd ../..`;
} else if (platform === "win32") {
  const cmakeArgs = process.env.CMAKE_TOOLCHAIN_FILE
    ? `-DCMAKE_TOOLCHAIN_FILE="${process.env.CMAKE_TOOLCHAIN_FILE}"`
    : "";
  await $`cd vendor/libxml2 && cmake . ${cmakeArgs} && cd ../..`;
} 
