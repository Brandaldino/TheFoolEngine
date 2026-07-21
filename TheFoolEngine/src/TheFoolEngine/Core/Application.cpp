#include "tfpch.h"
#include "Application.h"

#include "TheFoolEngine/Core/Log.h"

#include "TheFoolEngine/Renderer/Renderer.h"

#include "Input.h"

#include <glfw/glfw3.h>

#include "Job/JobSystem.h"

namespace TheFoolEngine 
{

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		TF_PROFILE_FUNCTION();

		TF_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Init();

        JobSystem::Initialize();

		m_ImGuiLayer = new ImGuiLayer;
		PushOverLayer(m_ImGuiLayer);
	}

	Application::~Application()
    {
        JobSystem::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
    {
		TF_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverLayer(Layer* layer) 
    {
		TF_PROFILE_FUNCTION();

		m_LayerStack.PushOverLayer(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		TF_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			if (e.Handled)
				break;
			(*--it)->OnEvent(e);
		}
	}

	void Application::Close() 
	{
		m_Running = false;
	}

	void Application::Run() 
    {
		TF_PROFILE_FUNCTION();

		while (m_Running) 
		{
			TF_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime();
			TimeStep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minized) 
			{
				{
					TF_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}

				m_ImGuiLayer->Begin();
				{
					TF_PROFILE_SCOPE("LayerStack OnImGuiRender");

					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}
				

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		TF_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0) 
        {
			m_Minized = true;
			return true;
		}
		m_Minized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}