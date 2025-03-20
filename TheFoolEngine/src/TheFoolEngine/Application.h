#pragma once

#include "Core.h"


namespace TheFoolEngine {
	class TheFoolEngine_API Application{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLENT
	Application* CreateApplication();
}




