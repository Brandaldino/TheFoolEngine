#include "Sandbox_Model.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TFE = TheFoolEngine;

Sandbox_Model::Sandbox_Model()
	: Layer("Sandbox_Model"), m_CameraController(1280.0f / 720.0f)
{
    m_DebugLights.push_back({ 0, {5,5,5}, {2,3,2}, {1,0.95f,0.9f}, 1.5f });
    m_DebugLights.push_back({ 0, {5,5,5}, {-1,0.5f,1}, {0.6f,0.6f,0.7f}, 0.5f });
    m_DebugLights.push_back({ 0, {5,5,5}, {0,-1,-2}, {0.4f,0.5f,0.6f}, 0.4f });
}

void Sandbox_Model::OnAttach() 
{
	TF_PROFILE_FUNCTION();

    TFE::PBRRenderer::Init();

    // skybox
    auto skybox = TFE::CubeMap::Create("assets/cubemap/alley.hdr");
    TFE::PBRRenderer::SetSkybox(skybox);

    // model
    TFE::PBRRenderProxy proxy;
    proxy.Model = TFE::CreateRef<TFE::PBRModel>();
    std::filesystem::path path = "assets/model/Test/Furina.fbx";
    // path = "assets/model/Test/MetalRoughSpheres.glb";
    // path = "assets/model/Test/BoxTextured.glb";
    // path = "assets/model/GLB/furina_02.glb";
    proxy.Model->Import(path);
    TFE::PBRRenderer::DefaultTextureFill(proxy.Model);
    proxy.Model->UpLoad();
    // register material
    auto& texSets = proxy.Model->GetModelData().Textures;
    for (auto& ts : texSets)
    {
        auto mat = TFE::CreateRef<TFE::PBRMaterial>();
        mat->SetMaterialData(ts);
        TFE::PBRMaterialBuilder builder;
        builder.Build(mat);
        TFE::PBRMaterialManager::Get()->RegisterMaterial(mat->GetHash(), mat);
    }

    m_PBRProxy = proxy;
}

void Sandbox_Model::OnDetach()
{
	TF_PROFILE_FUNCTION();
}

void Sandbox_Model::OnUpdate(TFE::TimeStep ts) 
{
	TF_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	// Render
    {
        TF_PROFILE_SCOPE("Renderer Prop");
        TFE::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        TFE::RenderCommand::Clear();
    }

    // === PBR Renderer ==========================
    TFE::PBRRenderer::ResetRendererState();
    {
        const auto& camera = m_CameraController.GetCamera();
        TFE::CameraData cameraData;
        cameraData.ViewMatrix = camera.GetViewMatrix();
        cameraData.ProjectionMatrix = camera.GetProjectionMatrix();
        cameraData.Position = camera.GetPosition();
        TFE::PBRRenderer::SetCamera(cameraData);

        // register entity
        TFE::PBRRenderer::Register(m_PBRProxy);

        // Lights
        for (auto& dl : m_DebugLights)
        {
            if (dl.Type == 0)
                TFE::PBRRenderer::AddLight(TFE::DirectionLight{
                    glm::normalize(dl.Direction), dl.Color, dl.Intensity
                });
            else if (dl.Type == 1)
                TFE::PBRRenderer::AddLight(TFE::PointLight{
                    dl.Position, dl.Color, dl.Intensity, dl.Range
                });
            else if (dl.Type == 2 && glm::length(dl.Direction) > 0.001f)
                TFE::PBRRenderer::AddLight(TFE::SpotLight{
                    dl.Position, glm::normalize(dl.Direction), dl.Color, dl.Intensity,
                    dl.Range, dl.InnerAngle, dl.OuterAngle
                });
        }

        TFE::PBRRenderer::Render();
    }
}

void Sandbox_Model::OnImGuiRender() 
{
	TF_PROFILE_FUNCTION();

    ImGui::Begin("Settings");

    auto stats = TFE::PBRRenderer::GetStats();
    ImGui::Text("PBRRenderer Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("MeshCount: %d", stats.MeshCount);

    ImGui::End();

    ImGui::Begin("Lights");

    const char* lightTypes[] = { "Directional", "Point", "Spot" };

    for (int i = 0; i < (int)m_DebugLights.size(); ++i)
    {
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Text("Light %d", i);

        auto& dl = m_DebugLights[i];
        ImGui::Combo("Type", &dl.Type, lightTypes, IM_ARRAYSIZE(lightTypes));

        if (dl.Type == 0 || dl.Type == 2)
            ImGui::DragFloat3("Direction", glm::value_ptr(dl.Direction), 0.05f);
        if (dl.Type == 1 || dl.Type == 2)
            ImGui::DragFloat3("Position", glm::value_ptr(dl.Position), 0.1f);

        ImGui::ColorEdit3("Color", glm::value_ptr(dl.Color));
        ImGui::SliderFloat("Intensity", &dl.Intensity, 0.0f, 20.0f);
        if (dl.Type == 1 || dl.Type == 2)
            ImGui::SliderFloat("Range", &dl.Range, 0.0f, 100.0f);
        if (dl.Type == 2)
        {
            ImGui::SliderAngle("Inner Angle", &dl.InnerAngle, 0.0f, 90.0f);
            ImGui::SliderAngle("Outer Angle", &dl.OuterAngle, 0.0f, 90.0f);
        }

        if (ImGui::Button("Remove"))
        {
            m_DebugLights.erase(m_DebugLights.begin() + i);
            ImGui::PopID();
            break;
        }

        ImGui::PopID();
    }

    if (ImGui::Button("+ Add Light"))
        m_DebugLights.emplace_back();

    ImGui::End();
}

void Sandbox_Model::OnEvent(TFE::Event& e) 
{
	m_CameraController.OnEvent(e);
}
