#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <commdlg.h>

#include "imguizmo/ImGuizmo.h"

namespace TheFoolEngine
{

    EditorLayer::EditorLayer()
        : Layer("EditorLayer"), m_PerspectiveCameraController(1280.0f / 720.0f)
    {
    }

    void EditorLayer::OnAttach() 
    {
        TF_PROFILE_FUNCTION();

        // Shader
        m_ToneMappingShader = Shader::Create("assets/shader/ToneMapping.glsl");
        m_BloomExtractShader = Shader::Create("assets/shader/BloomExtract.glsl");
        m_BloomBlurShader = Shader::Create("assets/shader/GaussianBlur.glsl");
        m_BloomCombineShader = Shader::Create("assets/shader/BloomCombine.glsl");
        m_FlatShader = Shader::Create("assets/shader/FlatColor.glsl");

        // Handle Create
        TextureDesc dirSpotDesc;
        dirSpotDesc.Width = SHADOWMAP_SIZE;
        dirSpotDesc.Height = SHADOWMAP_SIZE;
        dirSpotDesc.Type = RenderTargetType::DepthArray;
        dirSpotDesc.LayerCount = MAX_SHADOW_LIGHTS;
        dirSpotDesc.IsTransient = true;
        m_ShadowFBOHandle = m_RenderGraph.CreateRenderTarget(dirSpotDesc, "Dir&SpotShadow");

        TextureDesc pointDesc;
        pointDesc.Width = SHADOWMAP_SIZE;
        pointDesc.Height = SHADOWMAP_SIZE;
        pointDesc.Type = RenderTargetType::CubeMapArray;
        pointDesc.LayerCount = MAX_SHADOW_LIGHTS;
        pointDesc.IsTransient = true;
        m_PointShadowHandle = m_RenderGraph.CreateRenderTarget(pointDesc, "PointShadow");

        TextureDesc desc;
        desc.Width = 1280;
        desc.Height = 720;
        desc.Format = TextureFormat::RGBA16F;
        m_HDRHandle = m_RenderGraph.CreateRenderTarget(desc, "HDR");

        desc.Format = TextureFormat::RGBA8;
        m_LDRHandle = m_RenderGraph.CreateRenderTarget(desc, "LDR");

        // Bloom
        desc.Format = TextureFormat::RGBA16F;
        m_BloomAHandle = m_RenderGraph.CreateRenderTarget(desc, "BloomA");
        m_BloomBHandle = m_RenderGraph.CreateRenderTarget(desc, "BloomB");
        m_BloomCHandle = m_RenderGraph.CreateRenderTarget(desc, "BloomC");

        m_ActiveScene = CreateRef<Scene>();

        PBRRenderer::Init();

        m_ShadowRenderer = CreateRef<ShadowRenderer>();
        m_ShadowRenderer->Init();

        // SkyBox
        auto skybox = CubeMap::Create("assets/cubemap/space.hdr");
        PBRRenderer::SetSkybox(skybox);
        // Environment
        auto irradiance = IBLUtils::CreateIrradianceMap(skybox);
        auto prefilter = IBLUtils::CreatePrefilteredMap(skybox);
        auto brdfLUT = IBLUtils::CreateBRDFLUT(512);
        PBRRenderer::SetEnvironmentMap(irradiance, prefilter, brdfLUT);

        {
            // === Main light: Sun (Directional) ===================
            auto sun = m_ActiveScene->CreateEntity("Sun");
            sun.AddComponent<LightComponent>(LightComponent{
                0,                              // Type: Directional
                {0.0f, 0.0f, 0.0f},             // Position: Ignored for directional light
                glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)),   // Direction: from upper right front
                {1.0f, 0.95f, 0.9f},            // Color: warm white (key light)
                1.5f                            // Intensity
                });

            // === Point light: local accent (lighting focus) ======
            //auto pointLight = m_ActiveScene->CreateEntity("PointLight");
            //pointLight.AddComponent<LightComponent>(LightComponent{
            //    1,                              // Type: Point
            //    {0.0f, 4.0f, 0.0f},             // Position: above the model
            //    {0, -1, 0},                     // Direction: ignored for point light
            //    {1.0f, 0.8f, 0.6f},             // Color: warm orange (accent)
            //    300.0f,                         // Intensity: high for 1/d^2 falloff
            //    25.0f                           // Range: > distance to ground (14.5)
            //    });

            // === Point Light 1: main accent (above model) ==========
            auto pl1 = m_ActiveScene->CreateEntity("PointLight1");
            pl1.AddComponent<LightComponent>(LightComponent{
                1,
                {0.0f, 4.0f, 0.0f},              // Position:
                {0, -1, 0},                      // Direction: ignored
                {1.0f, 0.8f, 0.6f},              // Color: warm orange
                300.0f,                          // Intensity
                25.0f                            // Range
                });

            // === Point Light 2: front-left ========================
            auto pl2 = m_ActiveScene->CreateEntity("PointLight2");
            pl2.AddComponent<LightComponent>(LightComponent{
                1,
                {-2.0f, 3.0f, 1.0f},             // Position:
                {0, -1, 0},
                {0.6f, 0.8f, 1.0f},              // Color: cool blue (contrast)
                200.0f,                          // Intensity
                25.0f                            // Range
                });

            // === Point Light 3: front-right =======================
            //auto pl3 = m_ActiveScene->CreateEntity("PointLight3");
            //pl3.AddComponent<LightComponent>(LightComponent{
            //    1,
            //    {6.0f, 2.5f, 3.0f},              // Position:
            //    {0, -1, 0},
            //    {0.8f, 1.0f, 0.7f},              // Color: greenish
            //    200.0f,                          // Intensity
            //    25.0f                            // Range
            //    });

            //// === Point Light 4: rear, higher ======================
            //auto pl4 = m_ActiveScene->CreateEntity("PointLight4");
            //pl4.AddComponent<LightComponent>(LightComponent{
            //    1,
            //    {0.0f, 6.0f, -5.0f},             // Position:
            //    {0, -1, 0},
            //    {1.0f, 0.9f, 0.9f},              // Color: warm white
            //    250.0f,                          // Intensity
            //    30.0f                            // Range
            //    });

            // === Spot light: side fill for depth =================
            auto spotLight = m_ActiveScene->CreateEntity("SpotLight");
            spotLight.AddComponent<LightComponent>(LightComponent{
                2,                              // Type: Spot
                {-4.0f, 6.0f, -3.0f},           // Position: upper left side
                glm::normalize(glm::vec3(0.4f, -0.7f, 0.3f)),   // Direction: toward lower right
                {1.0f, 0.9f, 0.6f},             // Color: warm yellow
                55.0f,                          // Intensity
                30.0f,                          // Range: > distance to ground
                glm::radians(15.0f),            // InnerAngle
                glm::radians(35.0f)             // OuterAngle
                });

            // === Fill lights: brighten shadow areas (no shadow) ==
            auto fill1 = m_ActiveScene->CreateEntity("FillLight1");
            fill1.AddComponent<LightComponent>(LightComponent{
                0,
                {0,0,0},
                glm::normalize(glm::vec3(0.8f, -0.4f, 0.4f)),    // opposite fill
                {0.6f, 0.7f, 1.0f},             // Color: cool blue (contrast)
                0.4f
                });

            auto fill2 = m_ActiveScene->CreateEntity("FillLight2");
            fill2.AddComponent<LightComponent>(LightComponent{
                0,
                {0,0,0},
                glm::normalize(glm::vec3(0.2f, -0.3f, -0.9f)),   // rear fill
                {1.0f, 0.8f, 0.6f},             // Color: warm orange
                0.3f
                });
        }

        // Model
        m_PBRModel = CreateRef<PBRModel>();
        std::filesystem::path modelPath = "assets/model/Furina.fbx";
        m_PBRModel->Import(modelPath);
        PBRRenderer::DefaultTextureFill(m_PBRModel);
        m_PBRModel->UpLoad();

        auto modelEntity = m_ActiveScene->CreateEntity("Furina");
        modelEntity.AddComponent<PBRModelComponent>(m_PBRModel);

        // ground
        auto groundModel = CreateRef<PBRModel>();
        std::filesystem::path groundPath = "assets/model/BoxTextured.glb";
        groundModel->Import(groundPath);
        groundModel->SetBaseColor(glm::vec3(0.05f, 0.05f, 0.05f));
        PBRRenderer::DefaultTextureFill(groundModel);
        groundModel->UpLoad();

        auto& ground = m_ActiveScene->CreateEntity("Ground");
        ground.AddComponent<PBRModelComponent>(groundModel);

        auto& groundTransform = ground.GetComponent<TransformComponent>();
        groundTransform.Transform = glm::mat4(1.0f);
        groundTransform.Transform = glm::translate(groundTransform.Transform, glm::vec3(0.0f, -10.5f, 0.0f));
        groundTransform.Transform = glm::scale(groundTransform.Transform, glm::vec3(100.0f, 0.05f, 100.0f));

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

        // FlatColor
        m_OutlineVBO = VertexBuffer::Create(24 * sizeof(glm::vec3));
        m_OutlineVBO->SetLayout({
            {ShaderDataType::Float3, "a_Position"}
            });
        m_OutlineVAO = VertexArray::Create();
        m_OutlineVAO->AddVertexBuffer(m_OutlineVBO);

        // Pass init
        m_MainPass = CreateScope<MainPass>(PBRRenderer::GetPBRShader());
        m_MainPass->SetInputShadow(m_ShadowFBOHandle);
        m_MainPass->SetInputPointShadow(m_PointShadowHandle);
        m_MainPass->SetOutput(m_HDRHandle);
        
        m_ShadowPass = CreateScope<ShadowPass>();
        m_ShadowPass->SetOutput(m_ShadowFBOHandle);
        m_PointShadowPass = CreateScope<PointShadowPass>();
        m_PointShadowPass->SetOutput(m_PointShadowHandle);

        m_BloomExtractPass = CreateScope<BloomExtractPass>(m_BloomExtractShader);
        m_BloomExtractPass->SetInput(m_HDRHandle);
        m_BloomExtractPass->SetOutput(m_BloomAHandle);

        m_BloomBurPassH = CreateScope<BloomBlurPass>(m_BloomBlurShader);
        m_BloomBurPassV = CreateScope<BloomBlurPass>(m_BloomBlurShader);

        m_BloomBurPassH->SetInput(m_BloomAHandle);
        m_BloomBurPassH->SetOutput(m_BloomBHandle);

        m_BloomBurPassV->SetInput(m_BloomBHandle);
        m_BloomBurPassV->SetOutput(m_BloomCHandle);

        m_BloomCombinePass = CreateScope<BloomCombinePass>(m_BloomCombineShader);

        m_BloomCombinePass->SetInputHDR(m_HDRHandle);
        m_BloomCombinePass->SetInputBloom(m_BloomCHandle);
        m_BloomCombinePass->SetOutput(m_LDRHandle);

        m_ToneMappingPass = CreateScope<ToneMappingPass>(m_ToneMappingShader);

        m_ToneMappingPass->SetInput(m_HDRHandle);
        m_ToneMappingPass->SetOutput(m_LDRHandle);

        m_RenderGraph.AddPass(std::move(m_MainPass));
        m_RenderGraph.AddPass(std::move(m_PointShadowPass));
        m_RenderGraph.AddPass(std::move(m_ShadowPass));
        m_RenderGraph.AddPass(std::move(m_BloomExtractPass));
        m_RenderGraph.AddPass(std::move(m_BloomBurPassH));
        m_RenderGraph.AddPass(std::move(m_BloomBurPassV));
        m_RenderGraph.AddPass(std::move(m_BloomCombinePass));
        m_RenderGraph.AddPass(std::move(m_ToneMappingPass));
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
        if (FrameBufferSpecification spec = m_RenderGraph.GetFrameBuffer(m_LDRHandle)->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_PerspectiveCameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
            m_RenderGraph.Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // Update editor camera
        m_PerspectiveCameraController.OnUpdate(ts);

        // Render
        Renderer2D::ResetStats();

        // Update Scene
        m_ActiveScene->OnUpdate(ts, !m_Is3DMode);

        // PBR pass (editor camera)
        RenderContext context;
        context.ShadowRenderer = m_ShadowRenderer.get();
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
            context.Camera = cameraData;

            // Submit lights from ECS
            auto lightView = m_ActiveScene->m_Registry.view<LightComponent>();
            for (auto entity : lightView)
            {
                auto& lc = lightView.get<LightComponent>(entity);

                switch (lc.Type)
                {
                    case 0:
                    {
                        if (context.ShadowViewProjections.size() >= MAX_SHADOW_LIGHTS)
                            break;

                        int shadowIndex = (int)context.ShadowViewProjections.size(); 
                        context.ShadowViewProjections.push_back(ShadowMath::ComputeDirLightVP(glm::normalize(lc.Direction)));
                        context.ShadowRenderer->AddDirectionalLight(context, DirectionLight{ glm::normalize(lc.Direction), lc.Color, lc.Intensity }, shadowIndex);
                        break;
                    }
                    case 1:
                    {
                        if (context.ShadowViewProjections.size() >= MAX_SHADOW_LIGHTS)
                            break;

                        int shadowIndex = context.PointShadow.Count;
                        context.PointShadow.Lights[shadowIndex] = ShadowMath::ComputePointLightShadowData(lc.Position, 0.1f, 50.0f);
                        context.PointShadow.Count++;
                        context.ShadowRenderer->AddPointLight(context, PointLight{ lc.Position, lc.Color, lc.Intensity, lc.Range }, shadowIndex);
                        break;
                    }
                    case 2:
                    {
                        if (context.ShadowViewProjections.size() >= MAX_SHADOW_LIGHTS)
                            break;

                        int shadowIndex = (int)context.ShadowViewProjections.size();
                        context.ShadowViewProjections.push_back(
                            ShadowMath::ComputeSpotLightVP(
                                lc.Position,
                                glm::normalize(lc.Direction),
                                glm::degrees(lc.OuterAngle) * 2.0f
                            )
                        );
                        context.ShadowRenderer->AddSpotLight(
                            context,
                            SpotLight{ lc.Position, glm::normalize(lc.Direction),
                            lc.Color, lc.Intensity, lc.Range, lc.InnerAngle, lc.OuterAngle },
                            shadowIndex);
                        break;
                    }
                }
            }

            // Renderable
            auto pbrView = m_ActiveScene->m_Registry.view<TransformComponent, PBRModelComponent, TagComponent>();
            for (auto entity : pbrView)
            {
                auto& transform = pbrView.get<TransformComponent>(entity);
                auto& pbr = pbrView.get<PBRModelComponent>(entity);
                auto& tag = pbrView.get<TagComponent>(entity);
                PBRRenderProxy proxy;
                proxy.Model = pbr.Model;
                proxy.Transform = transform.Transform;
                proxy.Name = tag.Tag;
                context.Renderables.push_back(proxy);
            }

            m_ShadowRenderer->SetGPULightFBO(context);
            m_RenderGraph.Execute(context);
        }

        // FlatColor
        {
            if (auto selected = m_SceneHierarchyPanel.GetSelectionContext())
            {
                auto& modelData = m_SceneHierarchyPanel.GetSelectionContext().GetComponent<PBRModelComponent>().Model->GetModelData();
                glm::vec3 mergedMin(1e30f), mergedMax(-1e30f);

                for (auto& mesh : modelData.Meshes)
                {
                    mergedMin = glm::min(mergedMin, mesh.AABBMin);
                    mergedMax = glm::max(mergedMax, mesh.AABBMax);
                }

                // Model selection box
                glm::vec3 corners[8] = {
                    {mergedMin.x, mergedMin.y, mergedMin.z}, {mergedMax.x, mergedMin.y, mergedMin.z},
                    {mergedMin.x, mergedMax.y, mergedMin.z}, {mergedMax.x, mergedMax.y, mergedMin.z},
                    {mergedMin.x, mergedMin.y, mergedMax.z}, {mergedMax.x, mergedMin.y, mergedMax.z},
                    {mergedMin.x, mergedMax.y, mergedMax.z}, {mergedMax.x, mergedMax.y, mergedMax.z}
                };

                glm::vec3 lines[24];
                int edges[12][2] = {
                    {0,1},{1,3},{3,2},{2,0},
                    {4,5},{5,7},{7,6},{6,4},
                    {0,4},{1,5},{2,6},{3,7}
                };
                for (int i = 0;i < 12;++i)
                {
                    lines[i * 2] = corners[edges[i][0]];
                    lines[i * 2 + 1] = corners[edges[i][1]];
                }
                    m_OutlineVBO->SetData(lines, sizeof(lines));
                
                auto transform = m_SceneHierarchyPanel.GetSelectionContext().GetComponent<TransformComponent>().Transform;

                m_FlatShader->Bind();
                m_FlatShader->SetMat4("u_ViewProjection",
                    m_PerspectiveCameraController.GetCamera().GetProjectionMatrix() *
                    m_PerspectiveCameraController.GetCamera().GetViewMatrix()
                );
                m_FlatShader->SetMat4("u_Transform", transform);
                m_FlatShader->SetFloat4("u_Color", { 1.0f, 0.9f, 0.1f, 1.0f }); // yellow

                m_OutlineVAO->Bind();
                RenderCommand::DrawArrays(RendererAPI::DrawMode::Lines, 24);
            }
        }

        RenderCommand::SetDepthTest(RendererAPI::DepthTest::Off);
        RenderCommand::SetDepthTest(RendererAPI::DepthTest::On);
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
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
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
            m_PerspectiveCameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);
            m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
            m_RenderGraph.Resize(viewportPanelSize.x, viewportPanelSize.y);
        }
        uint32_t textureID = m_RenderGraph.GetTexture(m_LDRHandle)->GetRendererID();
        ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0,1 }, ImVec2{ 1,0 });

        // Converting screen coordinates back to local coordinates
        if (ImGui::IsMouseDoubleClicked(0) && m_ViewportHovered)
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetItemRectMin();
            ImVec2 localPos = { mousePos.x - windowPos.x, mousePos.y - windowPos.y };

            Ray::RayData ray = Ray::ScreenToRay(
                { localPos.x, localPos.y },
                m_ViewportSize.x, m_ViewportSize.y,
                m_PerspectiveCameraController.GetCamera().GetViewMatrix(),
                m_PerspectiveCameraController.GetCamera().GetProjectionMatrix()
            );

            // Iterate through all PBRModel entities for detection
            PickEntity(ray);
        }

        if (auto selected = m_SceneHierarchyPanel.GetSelectionContext())
        {
            auto& transform = selected.GetComponent<TransformComponent>().Transform;

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(
                ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                ImGui::GetWindowWidth(), ImGui::GetWindowHeight()
            );

            ImGuizmo::Manipulate(
                glm::value_ptr(m_PerspectiveCameraController.GetCamera().GetViewMatrix()),
                glm::value_ptr(m_PerspectiveCameraController.GetCamera().GetProjectionMatrix()),
                ImGuizmo::TRANSLATE,
                ImGuizmo::WORLD,
                glm::value_ptr(transform)
            );

            if (ImGuizmo::IsUsing())
                m_ViewportHovered = false;
        }
       
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event & e) 
    {
        if (ImGuizmo::IsUsing())
            return;

        m_PerspectiveCameraController.OnEvent(e);
    }

    void EditorLayer::PickEntity(const Ray::RayData& ray)
    {
        float nearDistance = 1e30f;
        Entity picked;

        auto view = m_ActiveScene->m_Registry.view<PBRModelComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& [model, transfer] = view.get<PBRModelComponent, TransformComponent>(entity);
            auto& modelData = model.Model->GetModelData();

            for (auto& mesh : modelData.Meshes)
            {
                glm::vec3 corners[8] = {
                    {mesh.AABBMin.x, mesh.AABBMin.y, mesh.AABBMin.z},
                    {mesh.AABBMin.x, mesh.AABBMin.y, mesh.AABBMax.z},
                    {mesh.AABBMin.x, mesh.AABBMax.y, mesh.AABBMin.z},
                    {mesh.AABBMin.x, mesh.AABBMax.y, mesh.AABBMax.z},
                    {mesh.AABBMax.x, mesh.AABBMin.y, mesh.AABBMin.z},
                    {mesh.AABBMax.x, mesh.AABBMin.y, mesh.AABBMax.z},
                    {mesh.AABBMax.x, mesh.AABBMax.y, mesh.AABBMin.z},
                    {mesh.AABBMax.x, mesh.AABBMax.y, mesh.AABBMax.z}
                };

                glm::mat4& t = transfer.Transform;
                glm::vec3 worldMin(1e30f), worldMax(-1e30f);
                for (auto& c : corners)
                {
                    glm::vec3 wc = glm::vec3(t * glm::vec4(c, 1.0f));
                    worldMin = glm::min(worldMin, wc);
                    worldMax = glm::max(worldMax, wc);
                }

                float dist;
                if (Ray::RayAABBIntersect(ray, worldMin, worldMax, dist))
                {
                    if (dist < nearDistance)
                    {
                        nearDistance = dist;
                        picked = Entity{ entity, m_ActiveScene.get() };
                    }
                }
            }
        }

        if (picked)
            m_SceneHierarchyPanel.SetSelectionContext(picked);
        else
            m_SceneHierarchyPanel.SetSelectionContext(Entity{});
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
