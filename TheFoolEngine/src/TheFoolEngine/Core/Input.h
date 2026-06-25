#pragma once

// Global Input

#include "TheFoolEngine/Core/Base.h"
#include "TheFoolEngine/Core/KeyCodes.h"
#include "TheFoolEngine/Core/MouseCodes.h"

namespace TheFoolEngine{

	enum CursorMode
	{
		Enabled,
		Disabled
	};

	class THEFOOLENGINE_API Input {
	public:
		static bool IsKeyPressed(KeyCode keycode); 
		static bool IsMouseButtonPressed(MouseCode button);

		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		// static void SetCursorMode(CursorMode mode);
	};

}
