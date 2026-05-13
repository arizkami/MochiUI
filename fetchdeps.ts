import { existsSync } from "node:fs";
import { join } from "node:path";

const root = import.meta.dir;

function hasArg(...names: string[]): boolean {
  return Bun.argv.slice(2).some((arg) => names.includes(arg));
}

async function run(command: string[], options: { cwd?: string } = {}): Promise<void> {
  console.log(`> ${command.join(" ")}`);
  const proc = Bun.spawn(command, {
    cwd: options.cwd ?? root,
    stdout: "inherit",
    stderr: "inherit",
    stdin: "inherit",
  });
  const code = await proc.exited;
  if (code !== 0) {
    throw new Error(`${command[0]} exited with code ${code}`);
  }
}

async function main(): Promise<void> {
  const skipSubmodules = hasArg("--no-submodules");
  const skipModules = hasArg("--no-modules");

  if (!skipSubmodules && existsSync(join(root, ".git"))) {
    await run(["git", "submodule", "sync", "--recursive"]);
    await run(["git", "submodule", "update", "--init", "--recursive"]);
  }

  if (!skipModules) {
    await run(["bun", "install"], { cwd: join(root, "modules") });
  }
}

main().catch((error) => {
  console.error(error instanceof Error ? error.message : error);
  process.exit(1);
});
