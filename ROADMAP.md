# TheFoolEngine 开发路线图（Roadmap）

## 进行中

- **LOD 框架**：LODGroup + 距离选择 + 场景序列化扩展（档位模型后续外部生成）
  - 背景：Furina 300 万索引全精度渲染，拉远后转动视野卡帧
  - 目标：离线多档 + 按距离选档（UE5 传统 Static Mesh LOD 思路）

## 已完成里程碑

- 视锥剔除（016）→ 合批/实例化（024）→ 遮挡剔除（025）——**渲染性能三级优化链完整**
- 阴影系统、动态 Pass、Shader 系统、多线程加载、纹理 mip、MSAA 等

## 规划 1：全局 UID 系统

**目标**：所有实体加入 uid，扩展为**整个引擎所有资产（模型/纹理/材质）统一的 uid 系统**。

**动机**：
- 遮挡剔除当前用 `entt::to_integral(entity)` 做键——实体销毁重建后 index 复用（版本号缓解但不绝对稳定），且无跨帧语义
- 序列化引用、资源管理、网络同步、调试都需要稳定 ID

**设计要点**：
- `IDComponent`（实体）：`uint64_t` 唯一 ID，创建时生成，场景保存/加载保留
- 资产 ID：模型/纹理/材质统一注册表（`AssetRegistry`），按 ID 引用而非路径
- 对齐 UE5：`UObject` + `FName`/`ObjectID` 体系
- 迁移：OcclusionManager 键改用 uid；场景序列化保存 uid

## 规划 2：跨图形 API 抽象（RHI）

**目标**：消除绕过抽象层直接调用 OpenGL API 的代码，为切换后端（Vulkan/DX12）铺路。

**现状问题**：大量直接 GL 调用散落在各层：
- `glNamedBufferStorage` / `glNamedBufferSubData`（SSBO）
- `glBindBufferRange` / `glBindTextureUnit` / `glColorMask`
- `glBeginQuery` / `glGetQueryObject*`（遮挡查询）
- `glCreateBuffers` 等 DSA 调用

**设计要点**：
- RHI 抽象层：Buffer/VAO/Texture/Shader/Query/SSBO 等资源全部走抽象接口
- 渲染器/Pass 只依赖抽象接口，不碰 GL 头
- GL 实现收进 `Platform/OpenGL/`，后续可加 `Platform/Vulkan/`
- 分步：先收新加的（SSBO、OcclusionQuery），再收历史遗留（DSA 调用、直接 gl*）
- 对齐 UE5：`RHI`（RHIResource/RHICmdList）分层

## 规划 3：HZB 遮挡剔除演进（遮挡剔除根治）

**目标**：从硬件遮挡查询（画包围盒 + 深度测试）演进为 **HZB（Hierarchical Z-Buffer）**——UE5 做法。

**动机**：
- 当前画包围盒 + 深度测试有**贴合问题**（扁平物体包围盒深度 ≈ 场景深度 → `GL_LESS` 误判），需 Polygon Offset 调参
- HZB 不画包围盒，直接采样深度 mip 判断——**从根上避免贴合问题**，且零包围盒 draw call

**设计要点**：
- 深度 FBO 生成 HZB（深度 mip 链，2x2 取最靠近相机的深度）
- 包围盒投影 → 选匹配 mip level → 采样判断是否被遮挡
- 保留现有分层状态机（Visible/RecentlyOccluded/Occluded）
- 前置：RHI 抽象完成（需要 HZB 生成 pass + 采样逻辑）

## 其他候选

- 阴影质量（2048/PCF，之前搁置）
- 真纹理流送、pass 并行调度