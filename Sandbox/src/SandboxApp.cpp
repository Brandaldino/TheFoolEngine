#include <TheFoolEngine.h> 

class Sandbox :public TheFoolEngine::Application {
public:
	Sandbox() {
	}
	~Sandbox() {

	}

};


TheFoolEngine::Application* TheFoolEngine::CreateApplication() {
	return new Sandbox();
}