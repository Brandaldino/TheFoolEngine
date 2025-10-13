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

void Sandbox_Model::OnAttach() {
	TF_PROFILE_FUNCTION();

	m_Container = TFE::Texture2D::Create("assets/textures/container2.png");
	m_ContainerSpecular = TFE::Texture2D::Create("assets/textures/container2_specular.png");

	m_Model_1 = TFE::CreateRef<TFE::ModelImporter>("assets/model/Obj/modelNew.obj");
	// m_Model_1 = TFE::CreateRef<TFE::ModelImporter>("assets/model/GLB/modelNew.glb");
	// m_Model_2 = TFE::CreateRef<TFE::ModelImporter>("assets/model/character.blend");

	m_MeshDatas = m_Model_1->GetMeshData();


	// white texture
	auto whiteTexture = TFE::Texture2D::Create(1, 1);
	uint32_t texData = 0xffffffff;
	whiteTexture->SetData(&texData, sizeof(uint32_t));

	auto blackTexture = TFE::Texture2D::Create(1, 1);
	uint32_t blackTexData = 0x00000000;
	blackTexture->SetData(&blackTexData, sizeof(uint32_t));

	auto normalTexture = TFE::Texture2D::Create(1, 1);
	uint32_t normalTexData = 0xffff8080;
	normalTexture->SetData(&normalTexData, sizeof(uint32_t));

	auto grayTexture = TFE::Texture2D::Create(1, 1);
	uint32_t grayTexData = 0xFF7F7F7F;
	grayTexture->SetData(&grayTexData, sizeof(uint32_t));

	auto builder = TFE::CreateRef<TFE::MaterialBuilder>();
	builder->AddTexture(TFE::TextureType::Diffuse, whiteTexture);      // 漫反射：白色
	builder->AddTexture(TFE::TextureType::Normal, normalTexture);      // 法线：默认法线
	builder->AddTexture(TFE::TextureType::Specular, blackTexture);     // 高光/金属度：黑色（无高光）
	builder->AddTexture(TFE::TextureType::Emissive, blackTexture);     // 自发光：黑色（不发光）
	builder->AddTexture(TFE::TextureType::Opacity, whiteTexture);      // 不透明度：白色（不透明）
	builder->AddTexture(TFE::TextureType::Height, blackTexture);       // 高度：黑色（无高度）
	builder->AddTexture(TFE::TextureType::Ambient, whiteTexture);      // 环境光遮蔽：白色（无遮蔽）
	builder->AddTexture(TFE::TextureType::Shininess, grayTexture);     // 光泽度/粗糙度：灰色（中等）
	builder->AddTexture(TFE::TextureType::Displacement, blackTexture); // 位移：黑色（无位移）
	builder->AddTexture(TFE::TextureType::Lightmap, whiteTexture);     // 光照贴图：白色
	builder->AddTexture(TFE::TextureType::Reflection, blackTexture);   // 反射：黑色（无反射）

	auto defaultMaterial = builder->Build();

	m_DefaultMaterialHash = defaultMaterial->GetHash();
}

void Sandbox_Model::OnDetach()
{
	TF_PROFILE_FUNCTION();

}

void Sandbox_Model::OnUpdate(TFE::TimeStep ts) {
	TF_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	// Render
	TFE::Renderer3D::ResetStats();
	{
		TF_PROFILE_SCOPE("Renderer Prop");
		TFE::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		TFE::RenderCommand::Clear();
	}

	TFE::MeshRenderer::ResetStats();

	{
		TF_PROFILE_SCOPE("Renderer Draw");

		// ------------------------- MeshRenderer Test ------------------------- //

		std::sort(m_MeshDatas.begin(), m_MeshDatas.end(),
			[](const TFE::Ref<TFE::MeshData>& a, const TFE::Ref<TFE::MeshData>& b) {
				return a->GetMaterialID() > b->GetMaterialID();
			});

		TFE::MeshRenderer::BeginScene(m_CameraController.GetCamera());

		TFE::MeshRenderer::AddLight(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(1.0f), 1000.0f, 1);

		auto size = m_MeshDatas.size();

		for (auto& it : m_MeshDatas)
		{
			uint32_t materialHash = TFE::MaterialManager::Get().GetHashByID(it->GetMaterialID());
			TFE::MeshRenderer::SubmitMeshData(glm::vec3(0.0f), glm::vec3(1.0f), m_DefaultMaterialHash, it->GetVertexArray(), it->GetIndexArray());
		}

		TFE::MeshRenderer::EndScene();

		// ------------------------- Sphere Test ------------------------- //
		// SphereTest();

	}

}

void Sandbox_Model::OnImGuiRender() {
	TF_PROFILE_FUNCTION();

	ImGui::Begin("Entity Counter");

	auto stats = TFE::MeshRenderer::GetStats();
	ImGui::Text("Renderer3D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Mesh Count: %d", stats.MeshCount);
	ImGui::Text("Vertices: %d", stats.CurrentVertexCount);
	ImGui::Text("Indices: %d", stats.CurrentIndexCount);

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox_Model::OnEvent(TFE::Event& e) {
	m_CameraController.OnEvent(e);
}



// ------------------------ text function ------------------------------
void Sandbox_Model::SphereTest()
{
	uint32_t latitudeSegments = 32;
	uint32_t longitudeSegments = 32;
	float radius = 1.0f;

	auto sphereVertex = std::vector<TFE::MeshVertexData>();
	auto sphereIndex = std::vector<uint32_t>();
	float PI = 3.1415926;

	for (uint32_t lat = 0; lat <= latitudeSegments; ++lat)
	{
		float phi = PI * lat / latitudeSegments;
		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);

		for (uint32_t lon = 0; lon <= longitudeSegments; ++lon)
		{
			float theta = 2 * PI * lon / longitudeSegments;
			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			TFE::MeshVertexData vertex;
			vertex.Position =
			{
				radius * sinPhi * cosTheta,
				radius * cosPhi,
				radius * sinPhi * sinTheta
			};

			vertex.Normal = glm::normalize(vertex.Position);

			vertex.TexCoords =
			{
				static_cast<float>(lon) / longitudeSegments,
				static_cast<float>(lat) / latitudeSegments
			};

			vertex.Tangent = glm::normalize(glm::vec3(
				-sinTheta, 0.0f, cosTheta
			));

			vertex.Bitangent = glm::cross(vertex.Normal, vertex.Tangent);

			sphereVertex.push_back(vertex);
		}
	}

	for (uint32_t lat = 0; lat < latitudeSegments; ++lat)
	{
		for (uint32_t lon = 0; lon < longitudeSegments; ++lon)
		{
			uint32_t first = lat * (longitudeSegments + 1) + lon;
			uint32_t second = first + longitudeSegments + 1;

			sphereIndex.push_back(first);
			sphereIndex.push_back(second);
			sphereIndex.push_back(first + 1);

			sphereIndex.push_back(first + 1);
			sphereIndex.push_back(second);
			sphereIndex.push_back(second + 1);
		}
	}

	TFE::MeshRenderer::BeginScene(m_CameraController.GetCamera());

	TFE::MeshRenderer::AddLight(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(1.0f), 1000.0f, 1);
	//TFE::MeshRenderer::AddLight(glm::vec3(10.0f, 10.0f, -10.0f), glm::vec3(1.0f), 1000.0f, 1);
	//TFE::MeshRenderer::AddLight(glm::vec3(-10.0f, 10.0f, -10.0f), glm::vec3(1.0f), 1000.0f, 1);
	//TFE::MeshRenderer::AddLight(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(1.0f), 1000.0f, 1);

	TFE::MeshRenderer::SubmitMeshData(glm::vec3(0.0f), glm::vec3(1.0f), m_DefaultMaterialHash, sphereVertex, sphereIndex);

	TFE::MeshRenderer::EndScene();
}