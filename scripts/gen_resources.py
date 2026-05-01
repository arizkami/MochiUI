import os
import sys
import xml.etree.ElementTree as ET


def generate_resources(xml_path, header_path, source_path):
    tree = ET.parse(xml_path)
    root = tree.getroot()
    base_dir = os.path.dirname(xml_path)

    # Generate Header
    with open(header_path, "w") as f:
        f.write("#pragma once\n\n")
        f.write("namespace AureliaUI {\n")
        f.write("    void InitBinaryResources();\n")
        f.write("} // namespace AureliaUI\n")

    # Generate Source
    with open(source_path, "w") as f:
        f.write(f'#include "{os.path.basename(header_path)}"\n')
        f.write("#include <core/ResourceManager.hpp>\n\n")
        f.write("namespace AureliaUI {\n\n")

        resources = []
        for file_tag in root.findall("file"):
            rel_path = file_tag.text.strip()
            abs_path = os.path.join(base_dir, rel_path)

            if not os.path.exists(abs_path):
                continue

            with open(abs_path, "rb") as res_file:
                content = res_file.read()

            var_name = "res_" + rel_path.replace("/", "_").replace(".", "_").replace(
                "-", "_"
            )
            f.write(f"static const uint8_t {var_name}[] = {{")
            f.write(",".join(hex(b) for b in content))
            f.write("};\n")
            resources.append((rel_path, var_name, len(content)))

        f.write("\nvoid InitBinaryResources() {\n")
        f.write("    static bool initialized = false;\n")
        f.write("    if (initialized) return;\n")
        f.write("    initialized = true;\n\n")
        f.write("    auto& rm = ResourceManager::getInstance();\n")
        for rel_path, var_name, size in resources:
            f.write(
                f'    rm.registerResource("res://{rel_path}", {var_name}, {size});\n'
            )
        f.write("}\n\n")

        # Add auto-initialization helper
        f.write("struct BinaryResourceInitializer {\n")
        f.write("    BinaryResourceInitializer() { InitBinaryResources(); }\n")
        f.write("};\n\n")
        f.write("static BinaryResourceInitializer g_binaryResourceInitializer;\n\n")

        f.write("} // namespace AureliaUI\n")


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print(
            "Usage: python gen_resources.py <resources.xml> <output.hpp> <output.cpp>"
        )
        sys.exit(1)
    generate_resources(sys.argv[1], sys.argv[2], sys.argv[3])
