#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <commdlg.h>

namespace TheFoolEngine
{

    EditorLayer::EditorLayer()
        : Layer("EditorLayer"), m_PerspectiveCameraController(1280.0f / 720.0f)
    {
    }

    void EditorLayer::OnAttach() 
    {
        TF_PROFILE_FUNCTION();

        FrameBufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_FrameBuffer = FrameBuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();

        PBRRenderer::Init();

        // SkyBox
        auto skybox = CubeMap::Create("assets/cubemap/alley.hdr");
        PBRRenderer::SetSkybox(skybox);
        // Environment
        auto irradiance = IBLUtils::CreateIrradianceMap(skybox);
        auto prefilter = IBLUtils::CreatePrefilteredMap(skybox);
        auto brdfLUT = IBLUtils::CreateBRDFLUT(512);
        PBRRenderer::SetEnvironmentMap(irradiance, prefilter, brdfLUT);

        // Lights
        auto dirLight = m_ActiveScene->CreateEntity("DirLight");
        dirLight.AddComponent<LightComponent>(LightComponent{ 0, {0,0,0}, {2,3,2}, {1,0.95f,0.9f}, 1.5f });

        auto pointLight = m_ActiveScene->CreateEntity("PointLight");
        pointLight.AddComponent<LightComponent>(LightComponent{ 1, {3,3,3}, {0,-1,0}, {1,0.3f,0.3f}, 2.0f, 15.0f });

        auto spotLight = m_ActiveScene->CreateEntity("SpotLight");
        spotLight.AddComponent<LightComponent>(LightComponent{ 2, {-3,2,0}, {1,-1,0}, {0.3f,1,0.3f}, 2.0f, 10.0f, glm::radians(10.0f), glm::radians(20.0f) });

        // Model
        m_PBRModel = CreateRef<PBRModel>();
        std::filesystem::path modelPath = "assets/model/MetalRoughSpheres.glb";
        modelPath = "assets/model/BoxTextured.glb";
        modelPath = "assets/model/Furina.fbx";
        m_PBRModel->Import(modelPath);
        PBRRenderer::DefaultTextureFill(m_PBRModel);
        m_PBRModel->UpLoad();

        auto modelEntity = m_ActiveScene->CreateEntity("Model");
        modelEntity.AddComponent<PBRModelComponent>(m_PBRModel);

        auto square = m_ActiveScene->CreateEntity("Green Square");
        square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

        m_SquareEntity = square;

        m_MainCamera = m_ActiveScene->CreateEntity("Camera Entity");
        m_MainCamera.AddComponent<CameraComponent>();

        m_SecondCamera = m_ActiveScene->CreateEntity("Clip-Camera Entity");
        m_SecondCamera.AddComponent<CameraComponent>().Primary = false;

        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate()
            {
                // TEST CODE
                auto& transform = GetComponent<TransformComponent>().Transform;
				transform[3][0] = rand() % 10 - 5.0f;
            }

            void OnDestroy()
            {
            }

            void OnUpdate(TimeStep ts)
            {
                auto& transform = GetComponent<TransformComponent>().Transform;
                float speed = 5.0f;

                if (Input::IsKeyPressed(Key::A))
                    transform[3][0] -= speed * ts;
                if (Input::IsKeyPressed(Key::D))
                    transform[3][0] += speed * ts;
                if (Input::IsKeyPressed(Key::W))
                    transform[3][1] += speed * ts;
                if (Input::IsKeyPressed(Key::S))
                    transform[3][1] -= speed * ts;
            }
        };

		m_MainCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDetach()
    {
        TF_PROFILE_FUNCTION();

    }

    void EditorLayer::OnUpdate(TimeStep ts) 
    {
        TF_PROFILE_FUNCTION();

        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

        // Resize
        if (FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_PerspectiveCameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Update editor camera
        m_PerspectiveCameraController.OnUpdate(ts);

        // Render
        Renderer2D::ResetStats();
        m_FrameBuffer->Bind();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        // Update Scene
        m_ActiveScene->OnUpdate(ts, !m_Is3DMode);

        // PBR pass (editor camera)
        if (m_Is3DMode)
        {
            CameraData cameraData;
            {
                const auto& cam = m_PerspectiveCameraController.GetCamera();
                cameraData.ViewMatrix = cam.GetViewMatrix();
                cameraData.ProjectionMatrix = cam.GetProjectionMatrix();
                cameraData.Position = cam.GetPosition();
            }

            PBRRenderer::ResetRendererState();
            PBRRenderer::SetCamera(cameraData);

            // Submit lights from ECS
            auto lightView = m_ActiveScene->m_Registry.view<LightComponent>();
            for (auto entity : lightView)
            {
                auto& lc = lightView.get<LightComponent>(entity);
                switch (lc.Type)
                {
                    case 0:
                        PBRRenderer::AddLight(DirectionLight{ glm::normalize(lc.Direction), lc.Color, lc.Intensity });
                        break;
                    case 1:
                        PBRRenderer::AddLight(PointLight{ lc.Position, lc.Color, lc.Intensity, lc.Range });
                        break;
                    case 2:
                        PBRRenderer::AddLight(SpotLight{ lc.Position, glm::normalize(lc.Direction), lc.Color, lc.Intensity, lc.Range, lc.InnerAngle, lc.OuterAngle });
                        break;
                }
            }

            auto pbrView = m_ActiveScene->m_Registry.view<TransformComponent, PBRModelComponent>();
            for (auto entity : pbrView)
            {
                auto& transform = pbrView.get<TransformComponent>(entity);
                auto& pbr = pbrView.get<PBRModelComponent>(entity);
                PBRRenderProxy proxy;
                proxy.Model = pbr.Model;
                proxy.Transform = transform.Transform;
                PBRRenderer::Register(proxy);
            }
            PBRRenderer::Render();
        }

        m_FrameBuffer->UnBind();
    }

    void EditorLayer::OnImGuiRender() 
    {
        TF_PROFILE_FUNCTION();

        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->GetWorkPos());
            ImGui::SetNextWindowSize(viewport->GetWorkSize());
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                // ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();

                if (ImGui::MenuItem("ImportModel"))
                    ImportModel();

                if (ImGui::MenuItem("ImportSkybox"))
                    ImportSkybox();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

		m_SceneHierarchyPanel.OnImGuiRender();

        ImGui::Begin("Settings");

        ImGui::Checkbox("3D Mode", &m_Is3DMode);

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Quad Count: %d", stats.QuadCount);
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

        if(m_SquareEntity)
        {
            ImGui::Separator();
            ImGui::Text("%s", m_SquareEntity.GetComponent<TagComponent>().Tag.c_str());


            auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
            ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
            ImGui::Separator();
        }

        ImGui::DragFloat3("Camera Transform",
            glm::value_ptr(m_MainCamera.GetComponent<TransformComponent>().Transform[3]));

        if (ImGui::Checkbox("Camera A", &m_PrimaryCamera))
        {
            m_MainCamera.GetComponent<CameraComponent>().Primary = m_PrimaryCamera;
            m_SecondCamera.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;
        }

        {
            auto& camera = m_SecondCamera.GetComponent<CameraComponent>().Camera;
            float orthoSize = camera.GetOrthographicSize();
            if (ImGui::DragFloat("Second Camera Ortho Size", &orthoSize))
                camera.SetOrthographicSize(orthoSize);
        }

        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize) && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
        {
            m_FrameBuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_PerspectiveCameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);
            m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        }
        // TF_WARN("Viewport Size: {0},{1}", viewportPanelSize.x, viewportPanelSize.y);
        uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
        ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event & e) 
    {
        m_PerspectiveCameraController.OnEvent(e);
    }

    void EditorLayer::ImportModel()
    {
        // win32 api
        OPENFILENAMEW ofn = {};
        wchar_t path[260] = {};

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = path;
        ofn.nMaxFile = 260;
        ofn.lpstrFilter = L"Model Files\0*.fbx;*.glb;*.gltf\0All Files\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST;

        if (!GetOpenFileNameW(&ofn))
            return;

        std::filesystem::path filepath(path);

        // load model
        auto model = CreateRef<PBRModel>();
        model->Import(filepath);
        if (model->GetModelData().Meshes.empty())
        {
            TF_ERROR("Failed to import model: {0}", filepath.u8string());
            return;
        }

        PBRRenderer::DefaultTextureFill(model);
        model->UpLoad();

        // create entity
        auto entity = m_ActiveScene->CreateEntity(filepath.stem().u8string());
        entity.AddComponent<PBRModelComponent>(model);
    }

    void EditorLayer::ImportSkybox()
    {
        // win32 api
        OPENFILENAMEW ofn = {};
        wchar_t path[260] = {};

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFile = path;
        ofn.nMaxFile = 260;
        ofn.lpstrFilter = L"Skybox Files\0*.hdr\0All Files\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (!GetOpenFileNameW(&ofn))
            return;

        std::filesystem::path filepath(path);

        // load skybox
        auto skybox = CubeMap::Create(path);
        PBRRenderer::SetSkybox(skybox);
        // environment
        auto irradiance = IBLUtils::CreateIrradianceMap(skybox);
        auto prefilter = IBLUtils::CreatePrefilteredMap(skybox);
        auto brdfLUT = IBLUtils::CreateBRDFLUT(512);
        PBRRenderer::SetEnvironmentMap(irradiance, prefilter, brdfLUT);
    }

}
