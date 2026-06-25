#include "tfpch.h"
#include "TestPBRPipeline.h"

#include "../../TheFoolEngine/src/TheFoolEngine"

namespace TheFoolEngine
{
    void TestPBRPipeline()
{
     1. 导入
    auto data = TheFoolEngineAssimpImporterImport(assetsmodelTestBoxTextured.glb);
    TF_CORE_INFO(Meshes {0}, Textures {1}, data.Meshes.size(), data.Textures.size());

     2. 构建+注册每个材质槽
    for (auto& texSet  data.Textures)
    {
        auto mat = TFECreateRefTFEPBRMaterial();
        mat-SetMaterialData(texSet);
        TFEMaterialBuilder builder;
        builder.Build(mat);
        TFEMaterialManagerGet().RegisterMaterial(mat-GetHash(), mat);

        TF_CORE_INFO(Material hash {0}, AlbedoMap {1}, NormalMap {2}, MRMap {3}, AOMap {4},
            mat-GetHash(),
            texSet.AlbedoMap  texSet.AlbedoMap-GetRendererID()  0,
            texSet.NormalMap  texSet.NormalMap-GetRendererID()  0,
            texSet.MetallicRoughnessMap  texSet.MetallicRoughnessMap-GetRendererID()  0,
            texSet.AOMap  texSet.AOMap-GetRendererID()  0);
    }

     3. 验证去重
    auto texSet0 = data.Textures[0];
    auto mat2 = TFECreateRefTFEPBRMaterial();
    mat2-SetMaterialData(texSet0);
    TFEMaterialBuilder builder2;
    builder2.Build(mat2);
    auto cached = TFEMaterialManagerGet().GetMaterial(mat2-GetHash());
    TF_CORE_ASSERT(cached != nullptr);
    TF_CORE_INFO(Cache hit {0}, cached-GetHash());
}
}