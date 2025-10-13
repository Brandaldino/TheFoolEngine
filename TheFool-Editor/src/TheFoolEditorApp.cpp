#include <TheFoolEngine.h>
#include <TheFoolEngine/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace TFE = TheFoolEngine;

class TheFoolEditor :public TheFoolEngine::Application {
public:
	TheFoolEditor()
		: Application("TheFool Editor")
	{
		PushLayer(new TFE::EditorLayer());
	}
	~TheFoolEditor() {

	}

};

TheFoolEngine::Application* TheFoolEngine::CreateApplication() {
	return new TheFoolEditor();
}