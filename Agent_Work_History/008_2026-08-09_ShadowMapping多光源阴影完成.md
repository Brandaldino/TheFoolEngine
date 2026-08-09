# 阶段性总结 — Shadow Mapping 多光源阴影完成

## 时间跨度
2026-08-09 前后

## 核心成就：从零实现 Shadow Mapping（方向光 + 多光源）

在已有 PBR/IBL/后处理管线基础上，完整实现了方向光阴影系统，并支持**多个方向光各自投影**（每光源一层阴影贴图）。

---

## 一、功能完成情况

### 阴影基础设施
| 功能 | 状态 | 说明 |
|------|------|------|
| DepthOnly Framebuffer | ✓ | 只挂深度附件，无颜色附件（`glDrawBuffer(GL_NONE)`） |
| 深度纹理格式 | ✓ | `AttachmentType` 枚举（Depth / DepthStencil）+ 跨 API 转换层 |
| 深度渲染 shader | ✓ | `DepthOnlyShader.glsl`（极简 vs/fs，只写深度） |
| 纹理数组深度附件 | ✓ | `GL_TEXTURE_2D_ARRAY`，多光源各占一层 |

### 多光源阴影
| 功能 | 状态 | 说明 |
|------|------|------|
| 每光源一层阴影贴图 | ✓ | `AttachLayer(layer)` 循环渲染 |
| GPULight.ShadowIndex | ✓ | std140 对齐（C++ 80 字节，含 padding） |
| 阴影矩阵数组 | ✓ | `u_ShadowMatrices[MAX_SHADOW_LIGHTS]` 传入 shader |
| 主 Pass 按索引采样 | ✓ | `u_ShadowMaps[gl.ShadowIndex]` 动态索引 sampler2DArray |
| PCF 软阴影 | ✓ | 3×3 邻域采样平均 |
| 多方向光各自投影 | ✓ | 场景 3 个方向光投影方向各不相同，已验证 |

### 已实现的光源类型
- 方向光（Directional）阴影 ✓

---

## 二、技术决策与关键实现

### 阴影管线数据流
```
每帧:
  ResetRendererState()       → 清空 LightViewProjections
  遍历 ECS 灯光:
    Type==0 方向光 → SetShadowLight(dir) → 返回递增 index
                   → AddLight(dir, index) → GPULight.ShadowIndex = index
  Register(renderables)
  RenderShadowPass()         → 每 index 一层，AttachLayer → 渲染深度
  Render()                   → 绑定纹理数组槽8 → u_ShadowMatrices 数组 → 按 ShadowIndex 采样
```

### 关键决策
- **纹理数组 vs 单张纹理**：选 `GL_TEXTURE_2D_ARRAY`，支持运行时动态索引（`sampler2D[32]` uniform 数组不支持循环变量索引）
- **std140 对齐**：`GPULight` 4×vec4 + int ShadowIndex + 12 字节 padding = 80 字节，与 GLSL 对齐
- **阴影矩阵每光源一个**：`LightViewProjections` 数组，CPU 传 `u_ShadowMatrices`，shader 按 `ShadowIndex` 动态索引
- **方向光用正交投影**：`glm::ortho` + `lookAt(lightPos, sceneCenter, up)`
- **深度图 border color = 1.0**：采样到边界视为"无阴影"

### 调试过程中的关键 Bug
| Bug | 根因 | 修复 |
|-----|------|------|
| FBO incomplete | GLenum/TextureFormat 混用，枚举值当内部格式 | 加 `AttachmentType` 重载 + 转换层 |
| 抽象类无法实例化 | `GetDepthArrayTextureID` 名字与基类纯虚不一致 | 统一命名 |
| 阴影消失 | 主 Pass 缺 `u_ShadowMatrix` 设置 | `Render()` 里补设矩阵 |
| 深度图全白 | `std::vector<mat4> v(count)` 预填充 + emplace_back 追加，矩阵错位 | 去掉预填充 |
| shader 编译失败 | `sampler2DArray` 的 `textureSize` 返回 ivec3 | `.xy` 显式取维度 |

---

## 三、未完成 / 待办

### 阴影功能扩展
- [ ] **聚光灯阴影**：透视投影深度 Pass（复用现有 FBO/shader 思路，改动小）
- [ ] **点光源阴影**：cubemap 6 面深度渲染（6 次 DrawCall/光源），shader 用 `samplerCube` + `direction` 采样
- [ ] **阴影距离 / CSM（Cascaded Shadow Maps）**：方向光近远分段，提高远处精度
- [ ] **阴影 map 分辨率可配置**：现在固定 1024×1024
- [ ] **阴影矩阵用场景包围盒中心**：现在 `SetShadowLight` 里 `sceneCenter` 硬编码原点

### 效果调优
- [ ] **阴影偏淡**：IBL 环境光（ambient）未被阴影衰减，阴影区域仍有环境光 → 阴影偏灰。是否让环境光也乘 `(1-shadow)` 需权衡
- [ ] **PCF 采样质量**：3×3 采样偏少，可扩展 5×5
- [ ] **bias 调优**：当前 0.005，过大缩小阴影范围，过小产生 acne
- [ ] **阴影边缘抗锯齿**：可考虑 PCF 权重或 Poisson 采样

### 代码清理
- [ ] **清理调试残留**：`PBRShader.glsl` 里多行注释掉的调试代码（projCoords 输出、`[0]` 固定索引等）
- [ ] **`PBRRenderer.cpp` 的调试日志**：`TF_CORE_WARN("DepthOnly RenderID: ...")` 待删
- [ ] **`MAX_SHADOW_LIGHTS` 与 LayerCount 一致性**：shader、CPU、FBO 三处上限要统一

---

## 四、相关文件

- `TheFoolEngine/src/TheFoolEngine/Renderer/PBRRenderer.h/.cpp`：SetShadowLight / RenderShadowPass / ShadowData
- `TheFoolEngine/src/TheFoolEngine/Renderer/FrameBuffer.h`：FrameBufferSpecification（DepthOnly, LayerCount）
- `TheFoolEngine/src/PlatForm/OpenGL/OpenGLFrameBuffer.h/.cpp`：纹理数组深度附件 + AttachLayer
- `TheFoolEngine/src/TheFoolEngine/Renderer/Texture.h`：AttachmentType 枚举
- `TheFoolEngine/src/PlatForm/OpenGL/OpenGLTexture.cpp`：AttachmentTypeTranslate / 重载构造函数
- `TheFoolEngine/src/TheFoolEngine/Renderer/Light.h`：GPULight + ShadowIndex + std140 padding
- `TheFool-Editor/src/EditorLayer.cpp`：多光源提交 + SetShadowLight/AddLight 桥接
- `assets/shader/DepthOnlyShader.glsl`：深度渲染 shader
- `assets/shader/PBRShader.glsl`：u_ShadowMaps / u_ShadowMatrices / CalculateShadow / ShadowIndex
- `TheFoolEngine/src/PlatForm/OpenGL/OpenGLShader.cpp`：SetMat4Array / UploadUniformMat4Array

---

## 五、经验教训

1. **跨 API 抽象不能把 GLenum 泄漏到抽象接口**——引擎层只暴露自己的枚举，转换在平台层内部
2. **std140 对齐是 GPU 数据结构的硬约束**——C++ 编译器不管 GLSL 规则，必须手动补 padding
3. **`sampler2DArray` 的 `textureSize` 返回 ivec3**（多一层维度），是常见的隐晦坑
4. **std::vector 预填充 + emplace_back 追加** 会导致数据错位，初始化要统一
5. **深度图"全白"往往是"没写入"而非"采样错"**——先确认纹理内容，再查采样逻辑
6. **用 RenderDoc 直接看纹理内容** 是最快的定位手段，比 shader 输出调试更高效
