# RenderGraph 完整实现清单

## 目标
将 PBRRenderer 的渲染管线重构为真正的 RenderGraph 系统：虚拟资源 + 依赖声明 + 拓扑排序 + 资源池复用。

## 架构总览

```
RenderContext（每帧输入）
    │
RenderGraph（调度器）
    ├── 收集 Pass 依赖（GetOutputs/GetInputs）
    ├── 拓扑排序
    ├── 从 ResourcePool 分配资源
    ├── 执行 Pass
    └── 帧末回收资源
    │
    ├── Pass 1: ShadowPass       （方向光/聚光阴影）
    ├── Pass 2: PointShadowPass  （点光源阴影）
    ├── Pass 3: MainPass         （PBR 主渲染 + skybox）
    ├── Pass 4: BloomExtractPass
    ├── Pass 5: BloomBlurPass
    ├── Pass 6: BloomCombinePass
    └── Pass 7: ToneMapPass
```

## 资源体系（虚拟资源 vs 实际资源）

```
TextureDesc（纯数据描述）──→  TextureHandle（虚拟句柄：Desc + PoolIndex）
                                    │
                                    ▼
                             ResourcePool（纹理池）
                                    │
                                    ▼
                             Ref<Texture2D>（实际 GL 纹理）
```

---

## 第 1 批：基础设施（已完成）

### 1.1 RenderGraphTypes.h ✓
- `TextureDesc`：资源描述（尺寸/格式/层数/mip/samples/标志）
- `operator==`：全字段比较
- `GetHash()`：全字段哈希
- `TextureDescHash`：哈希 functor（给 `unordered_map` 用）

### 1.2 TextureHandle.h ✓
- `Desc`：资源描述
- `PoolIndex`：唯一身份（资源池槽位）
- `Name`：调试用
- `operator==`：按 PoolIndex 比较（比"同一资源"而非"同 desc"）
- `GetHash()`：返回 PoolIndex
- `TextureHandleHash`：哈希 functor

### 1.3 ResourcePool.h/.cpp ✓
- `Allocate(desc)`：查空闲表复用 → 无则新建纹理
- `Release(poolIndex)`：放回空闲表
- `ResetFrame()`：帧末复位
- `GetRendererID(poolIndex)` / `GetTexture(poolIndex)`：查池

### 第 1 批待修（审查发现）
- [ ] 拼写：`GerRendererID` → `GetRendererID`
- [ ] `IsValid()`：PoolIndex 从 0 开始导致首纹理判错（改为 PoolIndex 从 1 起跳或 IsValid 用 UINT32_MAX）
- [ ] `Release` 加边界检查

---

## 第 2 批：依赖推导 + 调度（进行中）

### 2.1 依赖声明（Pass 层）
- [ ] `Pass` 提供 `GetOutputs()` / `GetInputs()` 返回 `std::vector<TextureHandle>`
- [ ] 每个具体 Pass 返回自己的输出/输入句柄

### 2.2 拓扑排序（RenderGraph 核心）
- [ ] 构建依赖图：pass 读的资源 = 哪个 pass 写的
- [ ] DFS 拓扑排序得到执行顺序
- [ ] 循环依赖检测（A 写 X，B 写 X 冲突；A 读 B 输出且 B 读 A 输出 → 报错）

### 2.3 资源分配
- [ ] 排序后为每个 pass 的 output 调 `m_Pool.Allocate(desc)`
- [ ] 记录 handle → PoolIndex 映射
- [ ] 执行后 `m_Pool.ResetFrame()` 或自动回收

### 2.4 验证
- [ ] 打印执行顺序，确认依赖正确
- [ ] 循环依赖能检测报错

---

## 第 3 批：具体 Pass 实现

### 3.1 MainPass
- [ ] `Execute`：m_Target->Bind() + PBR 实体渲染 + skybox
- [ ] 从 `PBRRenderer::Render(context)` 迁移逻辑

### 3.2 ShadowPass
- [ ] `Execute`：方向光/聚光深度渲染
- [ ] 从 `PBRRenderer::RenderShadowPass(context)` 迁移

### 3.3 PointShadowPass
- [ ] `Execute`：点光源 cubemap array 深度
- [ ] 从 `PBRRenderer::RenderPointShadowPass(context)` 迁移

### 3.4 BloomExtractPass
- [ ] `Execute`：bloom extract（读 HDR，写 BloomA）

---

## 第 4 批：后处理 Pass

### 4.1 BloomBlurPass（方向参数化，横/竖复用）
### 4.2 BloomCombinePass（HDR + Bloom → LDR）
### 4.3 ToneMapPass（HDR → LDR）

---

## 第 5 批：接入与瘦身

### 5.1 PBRRenderer 瘦身
- [ ] 删 `Render` / `RenderShadowPass` / `RenderPointShadowPass`
- [ ] `s_Data` 保留 Shader/DefaultTexture/Environment/State + FBO

### 5.2 EditorLayer 接入
- [ ] 用 RenderGraph 编排所有 pass
- [ ] 删除内联的 bloom/tonemap 逻辑

---

## 第 1-3 批完成状态（2026-08-19 更新）

### 已完成
- [x] ResourcePool（Allocate/Release/ResetFrame + unordered_map 复用）
- [x] TextureHandle（PoolIndex 身份 + operator== + GetHash）
- [x] RenderGraph::CreateTexture / RegisterTexture
- [x] RenderGraph::Execute（先分配 PoolIndex → 拓扑排序 → 执行 → ResetFrame）
- [x] 拓扑排序（Kahn 算法 + 循环依赖断言）
- [x] MainPass / ShadowPass / PointShadowPass 迁移完成（getter 注入访问 s_Data）
- [x] EditorLayer 接入 RenderGraph（OnAttach 构建一次 + OnUpdate 每帧 Execute）
- [x] 乱序 AddPass 验证通过（依赖推导正确）

### 关键设计/修复记录
1. **GetEnvironment 实现丢失** → 链接错误，补回
2. **pass 生命周期**：AddPass 只在 OnAttach 一次（move 后成员变空，每帧只 Execute）
3. **GetOutputs 返回值改引用**（`vector<TextureHandle>&`），让 RenderGraph 分配 PoolIndex 能写回
4. **Execute 顺序**：先分配 PoolIndex 再拓扑排序（否则依赖推导用 PoolIndex=0 全错）
5. **前向声明命名空间**：`class Shader;` 需在 `namespace TheFoolEngine` 内，否则声明 `::Shader` 实现 `TheFoolEngine::Shader` 冲突
6. **RenderGraph 跨帧存活**：成员变量，资源池才能跨帧复用

### PBRRenderer 新增 getter（pass 资源注入）
GetPBRShader / GetDefaultTexture / GetLightUBO / GetEnvironment / GetShadowFBO / GetDepthOnlyShader / GetShadowViewProjections / GetPointShadowDepthShader / GetPointShadowMap / GetPointShadowData

## 第 2 批补充：pass 资源来源设计（已讨论）

### 核心矛盾
- Pass 是独立类，无法访问 PBRRenderer 的 `s_Data`（匿名 namespace static）
- 但 shader/FBO/纹理都在 s_Data

### 解决方案：Getter 注入 + 每帧数据走 context

**原则**：
- A 类（跨帧资源：shader/FBO/纹理/环境）→ getter 注入
- B 类（每帧数据：阴影矩阵/点光数据）→ 点光走 getter（过渡），相机/图元/光源走 context

**Getter 清单**：
```cpp
// MainPass
static Ref<Shader> GetPBRShader();
static const PBRMaterialTextureSet& GetDefaultTexture();
static GLuint GetLightUBO();
static const EnvironmentData& GetEnvironment();
// ShadowPass
static Ref<FrameBuffer> GetShadowFBO();
static Ref<Shader> GetDepthOnlyShader();
static const std::vector<glm::mat4>& GetShadowViewProjections();
// PointShadowPass
static Ref<Shader> GetPointShadowDepthShader();
static Ref<PointShadowMap> GetPointShadowMap();
static const PointShadowData& GetPointShadowData();
```

**前置工作**：
- [ ] 4 个结构体（EnvironmentData/ShadowData/PointLightShadowData/PointShadowData）从 PBRRenderer.cpp 挪到 PBRRenderer.h
- [ ] MAX_SHADOW_LIGHTS 常量上移到头文件
- [ ] RenderContext 加 `Ref<ResourcePool> ResourcePool`

### Pass 接口原则（已定）
- 基类只放能力：`GetInputs/GetOutputs/Execute`
- 配置方法（SetTarget/SetThreshold）归各 pass 自己
- 资源注入：构造注入 shader + SetTarget 定目标 + handle 走池

---

## 提交建议（3 次）
1. **提交 1**：第 1 批（ResourcePool + TextureHandle）
2. **提交 2**：第 2 批（拓扑排序 + 资源分配）
3. **提交 3**：第 3-5 批（pass 迁移 + 接入）

## 关键验证标准
- 跨帧 GL 纹理 ID 稳定（池复用生效）
- 拓扑排序输出依赖正确的顺序
- 渲染画面与重构前一致（功能不回归）
