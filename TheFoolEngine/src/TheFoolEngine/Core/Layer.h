#pragma once

#include "TheFoolEngine/Core/Base.h"
#include "TheFoolEngine/Core/TimeStep.h"
#include "TheFoolEngine/Events/Events.h"

namespace TheFoolEngine {
	class THEFOOLENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};
}

