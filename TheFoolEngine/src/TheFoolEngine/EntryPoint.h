#pragma once



#ifdef TF_PLATFORM_WINDOWS

extern TheFoolEngine::Application* TheFoolEngine::CreateApplication();

int main(int argc,char** argv) {
	auto app = TheFoolEngine::CreateApplication();
	app->Run();
	delete app;
}


#endif