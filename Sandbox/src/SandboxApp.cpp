#include <TheFoolEngine.h>
#include <TheFoolEngine/Core/EntryPoint.h>

#include "PlatForm/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DemoGame/DemoGameLayer.h"

#include "Sandbox2D/Sandbox2D.h"

#include "Sandbox_Model/Sandbox_Model.h"

class Sandbox :public TheFoolEngine::Application 
{
public:
	Sandbox() 
    {
		// PushLayer(new GameLayer());
		
		// PushLayer(new Sandbox2D());
		PushLayer(new Sandbox_Model());
	}
	~Sandbox() 
    {
	}

};

TheFoolEngine::Application* TheFoolEngine::CreateApplication() 
{
	return new Sandbox();
}