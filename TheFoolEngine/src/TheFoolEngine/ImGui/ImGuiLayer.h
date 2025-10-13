#pragma once

#include "TheFoolEngine/Core/Layer.h"

#include "TheFoolEngine/Events/ApplicationEvents.h"
#include "TheFoolEngine/Events/MouseEvents.h"
#include "TheFoolEngine/Events/KeyBoardEvents.h"

namespace TheFoolEngine {

	class THEFOOLENGINE_API ImGuiLayer:public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}

