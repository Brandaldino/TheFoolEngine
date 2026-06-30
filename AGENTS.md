# TheFoolEngine - Agent Guide

## Build
- Generate VS2022 projects: `scripts/Win-GenProjects.bat`
- Open `TheFoolEngine.slnx` in VS2022
- Startup project: `Sandbox`

## Project Structure
### Engine (`TheFoolEngine/`)
- `src/TheFoolEngine/Core/` — Application, Layer, LayerStack, Window, Input, Log, Base, EntryPoint, TimeStep
- `src/TheFoolEngine/Events/` — Event base + typed subclasses (App, Window, Key, Mouse), EventDispatcher
- `src/TheFoolEngine/Renderer/` — Renderer2D/3D, MeshRenderer, RenderCommand, Camera, Shader, Texture, Material, MeshData, ModelImporter, Buffer, VertexArray, FrameBuffer
- `src/TheFoolEngine/Scene/` — Scene (entt::registry), Entity, Components (Tag, Transform, SpriteRenderer, Camera, NativeScript), ScriptableEntity
- `src/TheFoolEngine/ImGui/` — ImGuiLayer (docking + viewport)
- `src/TheFoolEngine/Debug/` — Instrumentor (Chrome trace format profiling)
- `src/PlatForm/Windows/` — WindowsWindow, WindowsInput (GLFW)
- `src/PlatForm/OpenGL/` — OpenGLContext, OpenGLRendererAPI, OpenGLBuffer, OpenGLVertexArray, OpenGLShader, OpenGLTexture, OpenGLFrameBuffer

### Sandbox (`Sandbox/`)
- `src/SandboxApp.cpp` — Entry point, picks active test layer
- `src/Sandbox2D/` — Batch 2D quad rendering test
- `src/Sandbox3D/` — Batch 3D cube/lighting test
- `src/Sandbox_Model/` — Model importing + mesh rendering test (currently active)
- `src/DemoGame/` + `src/DemoGameLib/` — Flappy Bird style game demo
- `src/Particle/` — 2D particle system

### Editor (`TheFool-Editor/`)
- `src/TheFoolEditorApp.cpp` — Entry point
- `src/EditorLayer.cpp/.h` — Core editor layer (FBO viewport, docking workspace, scene management)
- `src/Panels/SceneHierarchyPanel.cpp/.h` — Entity hierarchy tree panel

### Assets
- `assets/shaders/` — GLSL shaders (PBRLightShader.glsl, Lights.glsl)
- `assets/textures/`, `assets/model/` — runtime assets

### Vendor
- `vendor/GLFW/`, `vendor/Glad/`, `vendor/imgui/` — Built as StaticLib
- `vendor/spdlog/`, `vendor/glm/`, `vendor/entt/` — Header-only
- `vendor/stb_image/` — Compiled into engine directly
- `vendor/assimp/` — Prebuilt .lib linked externally

### Workspace Roots
- `Agent_Work_History/` — 对话历史备份，按 `[序号]_[日期]_[标题].md` 格式归档，用于上下文压缩时恢复
- `TFE_Test/` — 实验性代码/测试原型存放区，不影响主项目结构

## Conventions
- Namespace: `TheFoolEngine`, alias `TFE`
- Smart ptrs: `Ref<T>` = `shared_ptr`, `Scope<T>` = `unique_ptr` (from `Core/Base.h`)
- Logging: `TF_CORE_*` (engine), `TF_*` (client), via spdlog
- Assert: `TF_CORE_ASSERT`
- Premake: all project config in root `premake5.lua`
- Platform: Windows-only, OpenGL (4.1+ via GLAD)
- PCH: `tfpch.h` / `tfpch.cpp`
- C++ standard: C++17

## Architecture
- `Application` singleton -> owns `Window` + `LayerStack` + `ImGuiLayer`
- `EntryPoint.h` provides `main()`, client defines `CreateApplication()`
- Rendering: `Renderer2D`/`Renderer3D`/`MeshRenderer` (static batch APIs)
- ECS: EnTT via `Scene`/`Entity`/`Components.h`
- Events: typed hierarchy + `EventDispatcher`
- Materials: `MaterialBuilder` -> `Material` -> `MaterialManager` (singleton)
- Models: `ModelImporter` (Assimp) -> `MeshData`

## Testing
- Run Sandbox project to test engine features
- Active test layer is set in `SandboxApp.cpp`

## Agent Background
OpenCode 在此项目中扮演导师/引导者角色，辅助人类完成引擎功能拓展、架构设计、渲染性能优化、图形学研究等技术性磨砺。Agent 应当：
- 引导分析问题根因，而非直接给答案
- 指出工程上的利弊权衡，帮助理解为什么
- 保持技术深度，涉及图形学/性能/架构时尽量深入

## Agent Constraints
- **不得主动修改 `TheFoolEngine`、`Sandbox`、`TheFool-Editor` 中的源代码**，除非人类明确要求修改
- 可以读取、分析、建议，但修改权在人类手中
- AGENTS.md 本身除外（可应要求编辑）
- **UTF-8 encoding** — All file access, reading, and writing must use UTF-8 encoding.
- **Comments in English** — All code comments must be written in English only.
