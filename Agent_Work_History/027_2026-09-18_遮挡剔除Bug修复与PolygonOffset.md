# 遮挡剔除 Bug 修复：二分定位 + Polygon Offset

## 日期
2026-09-18

## 问题现象
- UI 切换 shadow 按钮 → Furina 变成无数不相接的小三角形（碎面）
- 画面残留：切换前的结果覆盖在切换后
- Ground 消失

## 二分定位（git 回退逐提交测试）

```
1883ea3（shadow 合批）— 正常
bdec323（遮挡剔除）— 异常 ← bug 在此引入
93aaa98（LOD framework）— 异常（继承）
```

**结论：遮挡剔除（bdec323）引入。**

## 根因分析（三个叠加）

### 1. OcclusionPass 用错 shader（包围盒矩阵无效）
OcclusionPass 用 `m_ShadowShader`（DepthOnlyShader）画包围盒——DepthOnlyShader 是**实例化的**（`u_InstanceModels[gl_InstanceID]` + `u_LightViewProjection`），但 OcclusionPass 里 `SetMat4("u_Model", box)` + 非实例化 `DrawIndexed` → **包围盒矩阵无效** → 遮挡查询全错。
**修复**：新建专用 `OcclusionShader.glsl`（`u_Projection/u_View/u_Model`），OcclusionPass 用它。

### 2. SetDepthWrite(Off) 状态泄漏（Furina 挖洞 + Ground 消失）
OcclusionPass 设 `SetDepthWrite(Off)`，**结尾只恢复 colorMask，没恢复 DepthWrite** → 下一帧 MainPass 深度写关闭 → 深度缓冲 = 上帧残留 → Furina 部分深度测试失败 → **挖洞**；Ground 大平面全失败 → **消失**。
**修复**：OcclusionPass 结尾恢复 `SetDepthWrite(On)` + `SetDepthFunc(Less)`（**状态隔离**）。
**教训**：每个 pass 用完必须恢复 GL 状态（深度测试/写入/掩码/多边形偏移）。

### 3. Ground 扁平包围盒深度贴合场景 → 误判遮挡
Ground（100×0.05×100 扁平地板）的 AABB 包围盒顶面和 MainPass 场景深度**只差 0.025**——深度精度下几乎重合 → `GL_LESS`（严格小于）失败 → 查询 0 → 误判遮挡 → Ground 消失。
**修复（方案 A）**：画包围盒时 `glEnable(GL_POLYGON_OFFSET_FILL)` + `glPolygonOffset(0, -36)`（深度向相机偏移，贴合物体通过 GL_LESS）。

## Polygon Offset 参数估算（非试错）

深度缓冲最小可分辨深度：
```
r ≈ z² × (1/near − 1/far) / 2^bits
```
本场景（near=0.01, far=1000, 24bit, z≈10.5）：r ≈ 0.0007。Ground 需偏移 0.025 → `0.025/0.0007 ≈ 36` → units = -36。

**原则**：偏移刚好让贴合物体通过，不破坏真遮挡（偏移极小，墙后物体仍被剔除）。

## 验证
- shadow 切换：Furina 不再碎面 ✓
- Ground 正常显示 ✓
- 方案 B（LEQUAL，零参数）也可用（保守），最终用方案 A（Polygon Offset，精确）

## 未来：HZB 演进（已入 ROADMAP）
UE5 真正做法：HZB（深度 mip 采样）——不画包围盒，采样深度判断遮挡，**从根上避免"画包围盒 vs 场景深度贴合"问题**。RHI 抽象完成后演进。