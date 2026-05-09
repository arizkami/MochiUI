import { mkdirSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { parse, compileScript, compileTemplate } from "@vue/compiler-sfc";

const moduleRoot = import.meta.dir;
const appPath = resolve(moduleRoot, "example", "App.vue");
const generatedDir = resolve(moduleRoot, ".generated");
const componentOut = resolve(generatedDir, "App.generated.ts");
const entryOut = resolve(generatedDir, "entry.ts");
const bundleOut = resolve(moduleRoot, "..", "..", "example", "VueUI", "bundle.js");
const scopeId = "Sphere-vue-demo";

mkdirSync(generatedDir, { recursive: true });
mkdirSync(dirname(bundleOut), { recursive: true });

const source = readFileSync(appPath, "utf8");
const parsed = parse(source, { filename: appPath });
if (parsed.errors.length > 0) {
  throw new Error(parsed.errors.map((error) => String(error)).join("\n"));
}

const script = compileScript(parsed.descriptor, {
  id: scopeId,
});
const template = compileTemplate({
  id: scopeId,
  filename: appPath,
  source: parsed.descriptor.template?.content ?? "",
  hoistStatic: false,
  compilerOptions: {
    bindingMetadata: script.bindings,
    runtimeModuleName: "@vue/runtime-core",
  },
});

if (template.errors.length > 0) {
  throw new Error(template.errors.map((error) => String(error)).join("\n"));
}

const normalizedScript = script.content
  .replaceAll('from "vue"', 'from "@vue/runtime-core"')
  .replace("export default", "const __sfc__ =");
const normalizedTemplate = template.code.replaceAll('from "vue"', 'from "@vue/runtime-core"');
writeFileSync(
  componentOut,
  `${normalizedScript}\n${normalizedTemplate}\n__sfc__.render = render;\nexport default __sfc__;\n`,
);

writeFileSync(
  entryOut,
  `import App from "./App.generated";\nimport { mountVueApp } from "../src/index";\nmountVueApp(App);\n`,
);

const build = Bun.spawnSync([
  "bun",
  "build",
  entryOut,
  "--outfile",
  bundleOut,
  "--format=iife",
  "--target=browser",
  "--minify-whitespace",
  "--define",
  "__VUE_OPTIONS_API__=false",
  "--define",
  "__VUE_PROD_DEVTOOLS__=false",
  "--define",
  "__VUE_PROD_HYDRATION_MISMATCH_DETAILS__=false",
], {
  stdio: ["inherit", "inherit", "inherit"],
});

if (build.exitCode !== 0) {
  process.exit(build.exitCode ?? 1);
}

rmSync(generatedDir, { recursive: true, force: true });
