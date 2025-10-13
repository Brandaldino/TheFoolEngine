#include "tfpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace TheFoolEngine{

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		TF_CORE_ASSERT(windowHandle, "Window Handle is null");
	}

	void OpenGLContext::Init() {
		TF_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		TF_CORE_ASSERT(status, "Failed to initialize Glad!");

		TF_CORE_INFO("OpenGL Renderer:");
		TF_CORE_INFO("  Vendor:	{0}", glGetString(GL_VENDOR));
		TF_CORE_INFO("  Renderer:	{0}", glGetString(GL_RENDERER));
		TF_CORE_INFO("  Version:	{0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		TF_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}
