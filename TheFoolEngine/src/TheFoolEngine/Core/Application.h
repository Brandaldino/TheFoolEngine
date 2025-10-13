#pragma once

#include "Base.h"

#include "Window.h"
#include "TheFoolEngine/Core/LayerStack.h"
#include "TheFoolEngine/Events/Events.h"
#include "TheFoolEngine/Events/ApplicationEvents.h"

#include "TheFoolEngine/Core/TimeStep.h"

#include "TheFoolEngine/ImGui/ImGuiLayer.h"

namespace TheFoolEngine {
	class Application{
	public:
		Application(const std::string& name = "TheFoolEngine App");
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverLayer(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minized = false;
		LayerStack m_LayerStack;
		TimeStep m_TimeStep;
		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}




