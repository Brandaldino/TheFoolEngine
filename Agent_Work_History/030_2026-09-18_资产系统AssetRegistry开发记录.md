# 资产系统开发记录（AssetRegistry，规划 1 Step 2）

## 日期
2026-09-18

## 目标
资产（模型/纹理/材质）统一 UID + 注册表（AssetRegistry）——按 UID 引用资产，替代路径字符串。对齐 UE5 的 `AssetRegistry` + 软硬引用思路。

## 设计

### 1. Asset 基类（Importer/Asset.h）
```cpp
enum class AssetType { Model, Texture, Material };

class Asset
{
    UUID m_ID;
    std::string m_Path;
    AssetType m_Type;
};

class ModelAsset : public Asset
{
    Ref<PBRModel> m_Model;
};
```
预留 Texture/Material 扩展。

### 2. AssetRegistry（单例）
```cpp
class AssetRegistry
{
    UUID Register(Ref<Asset> asset, const std::string& path);   // 同路径去重 → 返回现有 ID
    Ref<Asset> GetAsset(UUID id) const;
    UUID GetIDByPath(const std::string& path) const;
    Ref<Asset> GetByPath(const std::string& path) const;
    bool HasPath(const std::string& path) const;

    static std::string NormalizePath(const std::string& path);  // weakly_canonical
    static UUID HashPath(const std::string& path);              // 路径哈希 → 稳定 UID
};
```

### 3. 关键决策：资产 UID = 路径哈希（稳定身份）
- **实体 UID**（IDComponent）：**随机**——独一无二身份
- **资产 UID**：**路径哈希**（`std::hash<string>(NormalizePath)`）——同路径同 UID，跨保存加载稳定，无需持久化
- 语义区分：实体是"身份"（随机），资产是"稳定引用"（路径派生）

### 4. AsyncAssetLoader 整合
- m_Cache（path → model）**迁移到 AssetRegistry**
- LoadModelAsync：归一化 → registry 查已加载（直接回调）→ 未加载后台 Import
- ProcessCompleted：注册（HashPath UID）+ 并发去重（Register 返回现有 ID → 用现有的）

### 5. PBRModelComponent
```cpp
struct PBRModelComponent
{
    UUID ModelID = 0;                        // 主模型 UID（路径哈希）
    Ref<PBRModel> Model;
    std::vector<std::string> LODPaths;
    std::vector<float> LODDistances;
    std::vector<UUID> LODModelIDs;           // LOD 档 UID（对应 LODPaths）
    std::vector<Ref<PBRModel>> LODModels;
};
```
- **LOD 模型自动进 AssetRegistry**（LoadModelAsync 统一注册）
- 渲染仍用 `Ref<PBRModel>`（不查 registry），UID 是稳定引用

## 踩坑（重要）

1. **GetIDByPath 未归一化**：`find(path)` 应 `find(NormalizePath(path))`——Register 存归一化 key，查询不归一化则不一致（相对/绝对路径查询失败）
2. **m_Cache 迁移不彻底**：AsyncAssetLoader 整合后，`.h` 的 `m_Cache` 声明残留（死成员）——删掉
3. **AssetRegistry.h 误加 concurrentqueue include**（不需要）——保持头文件干净
4. **字段名 `UUID UUID`**：成员和类型同名（合法但混乱）——改 `ModelID`
5. **LODModelIDs 声明了但忘填充**：反序列化只填 ModelID/LODPaths——LOD 循环里补 `HashPath(lodPath)`

## 测试验证

| 测试 | 预期 |
|------|------|
| 回归渲染（Furina/Ground/阴影/LOD/遮挡）| 正常 |
| 实体 ID 保存/加载 | 一致（json IDComponent.ID）|
| 3 Furina ModelID | 相同（路径哈希）|
| LODModelIDs | 非空 |
| AssetRegistry 去重 | 3 Furina = 1 资产 |

全部通过。

## 后续（待办）

- TextureAsset/MaterialAsset 并入（AssetType 已预留）
- 手动创建模型（EditorLayer 测试实体）统一走 LoadModelAsync/注册（当前为既有行为，不走 registry）
- 序列化按 UID 引用（替代路径字符串）