# SphereKit — Work in Progress

> This project is currently under active development and is not yet stable.

![SphereKit UI preview](./assets/preview.png)

---

## Architecture overview

### Library dependency graph

```mermaid
flowchart BT
  subgraph ext [External / prebuilt]
    skia["Skia\n(D3D12 GPU backend)"]
    yoga["Yoga\n(flex layout engine)"]
    v8["V8 monolith\n(JavaScript runtime)"]
  end

  subgraph core [SphereKit.Foundation  ·  DLL]
    direction TB
    foundA["FlexNode · LayoutStyle · OverlayNode"]
    foundB["Win32Window · Application · ResourceManager"]
    foundC["ThemeManager · FocusManager · AnimationController"]
    foundD["Event structs · Win32Input · FlexRootDispatch"]
  end

  subgraph gi [SphereKit.GraphicInterface  ·  DLL]
    giA["SphereWidget · MenuBar · SphereUI ns"]
  end

  subgraph gc [SphereKit.GraphicComponent  ·  DLL]
    gcA["TextNode · ButtonNode · TextInput · SliderNode"]
    gcB["SwitchNode · ScrollAreaNode · CheckboxNode · …"]
  end

  subgraph jse [SphereKit.JavaScriptEngine  ·  DLL]
    jseA["V8 isolate · context · UiRegistry"]
    jseB["globalThis.SphereUI native bridge"]
  end

  subgraph jsui [Sphere.React / SphereKit.Vue  ·  DLL]
    reactA["RenderReactApp / RenderVueApp\n· takePendingRoot"]
  end

  subgraph fw [framework/MikoUI  ·  header-only]
    mikoA["MikoUI::Run — single-call React app launcher"]
  end

  skia  --> core
  yoga  --> core
  core  --> gi
  gi    --> gc
  gc    --> jse
  v8    --> jse
  jse   --> jsui
  gc    --> jsui
  jsui  --> fw
  gi    --> fw
```

---

### Two supported UI authoring paths

```mermaid
flowchart TB
  subgraph path1 [Path 1 · Native C++ UI Node]
    cpp["Build FlexNode tree\nin C++"]
  end

  subgraph path2 [Path 2 · React Native Like / Vue]
    tsx["React or Vue components\n(.tsx/.vue — modules/reactui or modules/vueui/)"]
    bun["bun build → bundle.js\n(IIFE, node target)"]
    genRes["gen_resources.py\n→ ReactUIResources.hpp / VueUIResources.hpp"]
    jsBridge["JavaScriptEngine\ninstallSphereUIGlobal\neval bundle"]
    auBridge["globalThis.SphereUI\ncreateNode · appendChild\nsetStyle · setCallback · setRoot"]
    registry["UiRegistry\nNodeId ↔ FlexNode::Ptr"]
    tsx --> bun --> genRes --> jsBridge
    jsBridge --> auBridge --> registry
  end

  subgraph runtime [Runtime — Win32Window]
    root["Root FlexNode tree\n(OverlayNode wrapper)"]
    win["Win32Window\nsetRoot → run"]
  end

  subgraph fw [MikoUI::Run · optional shortcut]
    miko["One-call helper:\ninit · eval · setRoot · run"]
  end

  cpp       --> root
  registry  --> root
  root      --> win
  path2     -.->|"via MikoUI::Run"| miko
  miko      --> win
```

---

### Per-frame rendering pipeline

```mermaid
flowchart LR
  subgraph loop [Win32 message loop]
    msg["WM_PAINT / WM_SIZE\nWM_LBUTTONDOWN · DBLCLK · UP\nWM_MOUSEMOVE · WHEEL\nWM_KEYDOWN · WM_CHAR"]
  end

  subgraph events [Event dispatch]
    w32i["Win32Input.hpp\nLPARAM → PointerEvent\n/ WheelEvent / KeyEvent"]
    disp["FlexRootDispatch.hpp\nbridges → FlexNode virtuals\nonMouseDown · onMouseMove\nonMouseWheel · onKeyDown · onChar"]
    hit["FlexNode hit-test\nframe.contains(x,y)\nreverse child order"]
    w32i --> disp --> hit
  end

  subgraph layout [Yoga layout  —  only when dirty]
    sync["syncSubtreeStyles()\nLayoutStyle → YGNodeStyleSet*\nflex shorthand · gap · padding"]
    calc["YGNodeCalculateLayout()\nYGDirectionLTR"]
    apply["applyYogaLayout()\nYGNodeLayoutGet* → FlexNode::frame\n(SkRect XYWH)"]
    sync --> calc --> apply
  end

  subgraph paint [Skia paint pass]
    clear["canvas->clear(TRANSPARENT)"]
    draw["FlexNode::draw(canvas)\ndrawSelf → background · borderRadius\nhover overlay"]
    scroll["ScrollAreaNode:\nclipRect · 2nd YGCalculate\nwith YGUndefined scroll axis\napplyYogaLayout + scrollY offset"]
    text["TextNode · ButtonNode · TextInput\nSwitchNode · SliderNode\nSkFont · drawSimpleText · drawRRect"]
    clear --> draw --> scroll
    draw --> text
  end

  subgraph gpu [D3D12 present]
    flush["GrDirectContext::flush()\n+ submit()"]
    pres["IDXGISwapChain::Present(0,0)\nno VSync — UI-thread paced"]
    flush --> pres
  end

  msg --> w32i
  msg --> layout
  layout --> paint
  paint --> flush
```

---

### JavaScript UI bridge detail

```mermaid
flowchart TB
  subgraph ts [TypeScript — modules/reactui/ + modules/vueui/]
    app["examples/App.tsx / example/App.vue\nReact 19 or Vue 3 components"]
    idx["src/index.ts\npublic entrypoint"]
    internals["src/core + src/payload + src/wrapper\nreconciler/renderer · payload types · wrappers"]
    idx --> internals
    app --> idx
  end

  subgraph v8side [V8 isolate — C++]
    glob["globalThis.SphereUI object\n8 native functions"]
    reg["UiRegistry\nnextId · nodes map\ncontentProxy (scroll transparent reroute)"]
    pend["pendingRoot → takePendingRoot()"]
    glob --> reg --> pend
  end

  subgraph nodes [FlexNode creation map]
    ntypes["View/SafeAreaView → Column\nrow → Row\nText → TextNode\nButton → ButtonNode\nTextInput → TextInput\nSwitch → SwitchNode\nSlider → SliderNode\nScrollView/FlatList → ScrollAreaNode\n  + inner content Column proxy"]
  end

  idx  -- "createNode(type)" --> glob
  idx  -- "appendChild(pid,cid)" --> glob
  idx  -- "setStyle(id,key,val)" --> glob
  idx  -- "setProp(id,key,val)" --> glob
  idx  -- "setCallback(id,event,fn)" --> glob
  idx  -- "setRoot(id)" --> glob
  glob --> ntypes
  pend --> RenderReactApp["RenderReactApp() / RenderVueApp()\nreturns FlexNode::Ptr root"]
```

**Scripted UI**: JavaScript runs in **V8** inside `SphereKit.JavaScriptEngine`. The engine installs `globalThis.SphereUI` before `eval`-ing the bundle. **`Sphere.React`** and **`SphereKit.Vue`** wrap this in `RenderReactApp` / `RenderVueApp`, which evaluate the bundle synchronously and return the root `FlexNode`. The TypeScript hosts now live under `modules/reactui/src/` and `modules/vueui/src/`, with demo entrypoints at `modules/reactui/examples/App.tsx` and `modules/vueui/example/App.vue`.

---

## Documentation

- **[docs/README.md](docs/README.md)** — full index of Markdown guides.
- **MDX copies** for static-site generators: **[docs/mdx/](docs/mdx/index.mdx)**

---

## Platform Support

### Windows ✅

Recommended: **Visual Studio 2022** or newer (MSVC x64), **CMake**, **Ninja**, **Python 3**.

Build from the repo root:

```powershell
.\build.ps1
```

The script sources MSVC via `vcvars64`, configures Ninja + Release if needed, and builds.

Prebuilts (Skia, optional V8) are downloaded or expected under `external/prebuilt/` — see [docs/BuildAndCI.md](docs/BuildAndCI.md).

---

### macOS / Linux ⚠️

Not yet available out of the box.
You must build the Skia render backend manually for cross-platform support.

Fetch Skia via:

```sh
git submodule update --init external/skia
```
