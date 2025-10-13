#pragma once



#ifdef TF_PLATFORM_WINDOWS

extern TheFoolEngine::Application* TheFoolEngine::CreateApplication();

int main(int argc,char** argv) {
	TheFoolEngine::Log::Init();

	TF_PROFILE_BEGIN_SESSION("Startup", "TheFoolEngineProfile-Startup.json");
	auto app = TheFoolEngine::CreateApplication();
	TF_PROFILE_END_SESSION();

	TF_PROFILE_BEGIN_SESSION("Runtime", "TheFoolEngineProfile-Runtime.json");
	app->Run();
	TF_PROFILE_END_SESSION();

	TF_PROFILE_BEGIN_SESSION("Shutdown", "TheFoolEngineProfile-Shutdown.json");
	delete app;
	TF_PROFILE_END_SESSION();
}


#endif