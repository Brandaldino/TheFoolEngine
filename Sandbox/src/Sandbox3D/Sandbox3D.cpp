#include "Sandbox3D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TFE = TheFoolEngine;

Sandbox3D::Sandbox3D()
	: Layer("Sandbox3D"), m_CameraController(1280.0f / 720.0f)
{
	m_LightPos = { 5.0f, 5.0f , 5.0f };
}

void Sandbox3D::OnAttach() {
	TF_PROFILE_FUNCTION();

    std::string path1 = "assets/textures/container2.png";
    std::string path2 = "assets/textures/container2_specular.png";
	m_Container = TFE::Texture2D::Create(path1);
	m_ContainerSpecular = TFE::Texture2D::Create(path2);
}

void Sandbox3D::OnDetach()
{
	TF_PROFILE_FUNCTION();

}

void Sandbox3D::OnUpdate(TFE::TimeStep ts) {
	TF_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	static float rotation = 0.0f;
	rotation += ts * 50.0f;

	// model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	projection = glm::perspective(glm::radians(-45.0f), 1280.f / 720.0f, 0.1f, 100.0f);

	// Render
	TFE::Renderer3D::ResetStats();
	{
		TF_PROFILE_SCOPE("Renderer Prop");
		TFE::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		TFE::RenderCommand::Clear();
	}

	{
		TF_PROFILE_SCOPE("Renderer Draw");

		static float rotationAngle = 0.0f;
		// rotationAngle += ts * 50.0f; 

		glm::mat4 cubeRotation = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
		cubeRotation = cubeRotation * glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 radians = glm::vec3(rotationAngle);

		std::array<glm::vec4, 6> color =
		{
			glm::vec4{ 0.8f, 0.2f, 0.3f, 1.0f },
			glm::vec4{ 0.2f, 0.8f, 0.3f, 1.0f },
			glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f },
			glm::vec4{ 0.8f, 0.8f, 0.2f, 1.0f },
			glm::vec4{ 0.8f, 0.2f, 0.8f, 1.0f },
			glm::vec4{ 0.2f, 0.8f, 0.8f, 1.0f }
		};

		glm::vec3 positions[7]
		{
			glm::vec3{ 0.0f, 0.0f, 0.0f},		// Test - target_entity
			glm::vec3{ 0.5f, 0.5f, -0.5f},		// Test - light_entity
			glm::vec3{ 0.0f, -0.5f, 0.0f},		// Test - target_entity2
			glm::vec3{ 0.2f, 0.3f, 0.8f},
			glm::vec3{ 0.8f, 0.8f, 0.2f},
			glm::vec3{ 0.8f, 0.2f, 0.8f},
			glm::vec3{ 0.2f, 0.8f, 0.8f}
		};



		// Ligth Entity
		glm::vec4 lsColor = { 1.0f, 1.0f , 1.0f , 1.0f };	//	The color of each face of the light source cube
		std::array<glm::vec4, 6> lightColor;
		lightColor.fill(lsColor);

		glm::vec4 singlecolor = { 1.0f, 0.0f, 0.0f, 1.0f };

		glm::vec3 pointLightPos[] = {
			glm::vec3(-5.0f,	5.0f,	-5.0f),
			glm::vec3(5.0F,		5.0f,	5.0f),
			glm::vec3(5.0f,		5.0f,	-5.0f),
			glm::vec3(-5.0f,	5.0f,	5.0f)
		};
		// Target Entity

		TFE::Renderer3D::BeginScene(m_CameraController.GetCamera(),glm::vec3(1.0f));
		// TFE::Renderer3D::BeginScene(m_CameraController.GetCamera());
		for (int j = 0; j < 7; ++j)
		{
			TFE::Renderer3D::DrawCube(positions[j] * 3.0f, glm::vec3(1.0f), m_Container, glm::vec3(10.0f * j));
		}
		TFE::Renderer3D::EndScene();

		// Light Entity
		TFE::Renderer3D::BeginScene(m_CameraController.GetCamera(), true);
		for (int i = 0; i < 4; ++i)
			TFE::Renderer3D::DrawCube(pointLightPos[i] * 0.5f, glm::vec3(0.2f), lightColor, radians);
		TFE::Renderer3D::EndScene();

	}

}

void Sandbox3D::OnImGuiRender() {
	TF_PROFILE_FUNCTION();

	ImGui::Begin("Entity Counter");

	auto stats = TFE::Renderer3D::GetStats();
	ImGui::Text("Renderer3D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quad Count: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

	ImGui::End();
}

void Sandbox3D::OnEvent(TFE::Event& e) {
	m_CameraController.OnEvent(e);
}
