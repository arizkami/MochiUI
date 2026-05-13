import { existsSync } from "node:fs";
import { cpus } from "node:os";
import { join } from "node:path";

const root = import.meta.dir;
const buildDir = join(root, process.env.SPHERE_BUILD_DIR ?? "build");
const config = process.env.CMAKE_BUILD_TYPE ?? (hasArg("--debug") ? "Debug" : "Release");
const cmake = process.env.SPHERE_CMAKE ?? "cmake";

const frameworkTargets = [
  "SphereKit_Foundation",
  "SphereKit_GraphicInterface",
  "SphereKit_GraphicComponent",
  "SphereKit_JavaScriptEngine",
  "Sphere_React",
  "Sphere_Vue",
  ...(process.platform === "win32" ? ["SphereKit_DirectAudioEngine"] : []),
];

const exampleTargets = [
  "SphereExample",
  "ActivityMonitor",
  "MixerConsole",
  "BorderlessPlayer",
  "ReactUIDemo",
  "VueUIDemo",
];

function args(): string[] {
  return Bun.argv.slice(2);
}

function hasArg(...names: string[]): boolean {
  return args().some((arg) => names.includes(arg));
}

function jobs(): string {
  return process.env.NUMBER_OF_PROCESSORS ?? String(cpus().length || 4);
}

async function run(command: string[], options: { cwd?: string; env?: Record<string, string> } = {}): Promise<void> {
  console.log(`> ${command.join(" ")}`);
  const proc = Bun.spawn(command, {
    cwd: options.cwd ?? root,
    env: { ...process.env, ...options.env },
    stdout: "inherit",
    stderr: "inherit",
    stdin: "inherit",
  });
  const code = await proc.exited;
  if (code !== 0) {
    throw new Error(`${command[0]} exited with code ${code}`);
  }
}

async function importMsvcEnvironment(): Promise<void> {
  if (process.platform !== "win32" || process.env.VSCMD_ARG_TGT_ARCH) return;

  const candidates = [
    "C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
  ];
  const vcvars = candidates.find(existsSync);
  if (!vcvars) return;

  const psCommand = `$envLines = cmd /c '"${vcvars}" >nul && set'; $envLines`;
  const proc = Bun.spawn(["powershell.exe", "-NoProfile", "-Command", psCommand], {
    stdout: "pipe",
    stderr: "inherit",
  });
  const output = await new Response(proc.stdout).text();
  const code = await proc.exited;
  if (code !== 0) {
    throw new Error("vcvars64.bat failed");
  }

  for (const line of output.split(/\r?\n/)) {
    const eq = line.indexOf("=");
    if (eq > 0) process.env[line.slice(0, eq)] = line.slice(eq + 1);
  }
}

async function configure(): Promise<void> {
  await importMsvcEnvironment();
  const generator = process.env.CMAKE_GENERATOR ?? "Ninja";
  await run([
    cmake,
    "-S",
    root,
    "-B",
    buildDir,
    "-G",
    generator,
    `-DCMAKE_BUILD_TYPE=${config}`,
  ]);
}

async function ensureConfigured(): Promise<void> {
  if (hasArg("--configure") || !existsSync(join(buildDir, "CMakeCache.txt"))) {
    await configure();
  } else {
    await importMsvcEnvironment();
  }
}

async function buildTargets(targets: string[]): Promise<void> {
  await ensureConfigured();
  for (const target of targets) {
    await run([cmake, "--build", buildDir, "--target", target, "--config", config, "-j", jobs()]);
  }
}

async function buildJs(): Promise<void> {
  await run(["bun", "run", "build"], { cwd: join(root, "modules", "reactui") });
  await run(["bun", "run", "build"], { cwd: join(root, "modules", "vueui") });
  await run(["bun", "run", "bundle"], { cwd: join(root, "modules", "reactui") });
  await run(["bun", "run", "bundle"], { cwd: join(root, "modules", "vueui") });
  await run(["bun", "run", "bundle:demo"], { cwd: join(root, "modules", "reactui") });
  await run(["bun", "run", "bundle:demo"], { cwd: join(root, "modules", "vueui") });
}

function requestedTargets(): string[] {
  const targetIndex = args().findIndex((arg) => arg === "--target" || arg === "-t");
  if (targetIndex >= 0 && args()[targetIndex + 1]) {
    return [args()[targetIndex + 1]];
  }
  if (hasArg("--framework") || hasArg("framework")) return frameworkTargets;
  if (hasArg("--example", "--examples") || hasArg("example", "examples")) return exampleTargets;
  return [...frameworkTargets, ...exampleTargets];
}

async function main(): Promise<void> {
  if (hasArg("configure")) {
    await configure();
    return;
  }

  if (hasArg("js", "--js")) {
    await buildJs();
    return;
  }

  if (!hasArg("--no-js") && !hasArg("--framework") && !hasArg("framework")) {
    await buildJs();
  }

  await buildTargets(requestedTargets());
}

main().catch((error) => {
  console.error(error instanceof Error ? error.message : error);
  process.exit(1);
});
