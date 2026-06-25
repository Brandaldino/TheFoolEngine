#include "Sandbox_Model.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TFE = TheFoolEngine;

Sandbox_Model::Sandbox_Model()
	: Layer("Sandbox_Model"), m_CameraController(1280.0f / 720.0f)
{
	m_LightPos = { 5.0f, 5.0f , 5.0f };
}

void Sandbox_Model::OnAttach() 
{
	TF_PROFILE_FUNCTION();

    TFE::PBRRenderer::Init();

    TFE::PBRRenderProxy proxy;
    proxy.Model = TFE::CreateRef<TFE::PBRModel>();
    std::filesystem::path path = "assets/model/Test/Furina.fbx";
    path = "assets/model/Test/MetalRoughSpheres.glb";
    proxy.Model->Import(path);
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
        TFE::Light light;
        light.Position = glm::vec3(1000.0f, 1000.0f, -10.0f);
        light.Color = glm::vec3(1.0f);
        light.Intensity = 100.0f;

        m_TotalTime += ts;
        float radius = 5.0f;
        float theta = m_TotalTime * 0.5f;
        float phi = m_TotalTime * 0.2f;
        light.Position.x = cos(theta) * cos(phi) * radius;
        light.Position.y = sin(phi) * radius;
        light.Position.z = sin(theta) * cos(phi) * radius;
        TFE::PBRRenderer::SetLight(light);

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

    // ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

    ImGui::End();
}

void Sandbox_Model::OnEvent(TFE::Event& e) 
{
	m_CameraController.OnEvent(e);
}
