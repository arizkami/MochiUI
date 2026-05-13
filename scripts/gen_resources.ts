import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { basename, dirname, join } from "node:path";

function usage(): never {
  console.error(
    "Usage: bun run scripts/gen_resources.ts <resources.xml> <output.hpp> <output.cpp> [InitFunctionName]",
  );
  process.exit(1);
}

function decodeXmlText(value: string): string {
  return value
    .replace(/&lt;/g, "<")
    .replace(/&gt;/g, ">")
    .replace(/&quot;/g, '"')
    .replace(/&apos;/g, "'")
    .replace(/&amp;/g, "&");
}

function resourceFiles(xml: string): string[] {
  const files: string[] = [];
  const filePattern = /<file\b[^>]*>([\s\S]*?)<\/file>/gi;
  for (const match of xml.matchAll(filePattern)) {
    const file = decodeXmlText(match[1].trim());
    if (file.length > 0) files.push(file);
  }
  return files;
}

function variableName(path: string): string {
  return `res_${path.replace(/[\/\\.\-]/g, "_").replace(/[^A-Za-z0-9_]/g, "_")}`;
}

function bytesLiteral(bytes: Uint8Array): string {
  return Array.from(bytes, (byte) => `0x${byte.toString(16)}`).join(",");
}

function generateResources(xmlPath: string, headerPath: string, sourcePath: string, initFn: string): void {
  const xml = readFileSync(xmlPath, "utf8");
  const baseDir = dirname(xmlPath);
  const resources: Array<{ relPath: string; varName: string; size: number }> = [];

  mkdirSync(dirname(headerPath), { recursive: true });
  mkdirSync(dirname(sourcePath), { recursive: true });

  writeFileSync(
    headerPath,
    `#pragma once\n\nnamespace SphereUI {\n    void ${initFn}();\n} // namespace SphereUI\n`,
  );

  let source = `#include "${basename(headerPath)}"\n`;
  source += "#include <core/ResourceManager.hpp>\n\n";
  source += "namespace SphereUI {\n\n";

  for (const relPath of resourceFiles(xml)) {
    const absPath = join(baseDir, relPath);
    let content: Uint8Array;
    try {
      content = readFileSync(absPath);
    } catch {
      continue;
    }

    const varName = variableName(relPath);
    source += `static const uint8_t ${varName}[] = {${bytesLiteral(content)}};\n`;
    resources.push({ relPath, varName, size: content.length });
  }

  source += `\nvoid ${initFn}() {\n`;
  source += "    static bool initialized = false;\n";
  source += "    if (initialized) return;\n";
  source += "    initialized = true;\n\n";
  source += "    auto& rm = ResourceManager::getInstance();\n";
  for (const resource of resources) {
    source += `    rm.registerResource("res://${resource.relPath}", ${resource.varName}, ${resource.size});\n`;
  }
  source += "}\n\n";

  if (initFn === "InitBinaryResources") {
    source += "struct BinaryResourceInitializer {\n";
    source += `    BinaryResourceInitializer() { ${initFn}(); }\n`;
    source += "};\n\n";
    source += "static BinaryResourceInitializer g_binaryResourceInitializer;\n\n";
  }

  source += "} // namespace SphereUI\n";
  writeFileSync(sourcePath, source);
}

const [, , xmlPath, headerPath, sourcePath, initFn = "InitBinaryResources"] = Bun.argv;
if (!xmlPath || !headerPath || !sourcePath) usage();

generateResources(xmlPath, headerPath, sourcePath, initFn);
