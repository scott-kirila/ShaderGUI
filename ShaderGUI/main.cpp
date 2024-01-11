#include "callbacks.h"
//#include "render_triangle.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"


#include <iostream>
#include <string>

GLuint VAO, VBO, FBO, RBO, texture_id, shaderProgram;

const char* vertexShaderSource =
R"(#version 330 core
	layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoords;
    out vec2 TexCoords;
	void main()
	{
	   gl_Position = vec4(aPos, 1);
       TexCoords = aTexCoords;
	})";

const char* fragmentShaderSource =
R"(#version 330 core
    in vec2 TexCoords;
	out vec4 FragColor;

    uniform vec2 ViewportSize;
    uniform float Time;

	void main()
	{
		FragColor = vec4(vec3(0), 1);
	})";

const int viewWidth = 1920;
const int viewHeight = 1080;

ImVec2 framebufferSize = ImVec2(viewWidth, viewHeight);

std::string glsl_version = "#version 330";

namespace ImGui {
	bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags | ImGuiInputTextFlags_CallbackResize, text_resize_callback, (void*)str);
	}
}

//void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
//    glViewport(0, 0, width, height);
//}

int main() {

	// GLFW
	glfwSetErrorCallback(glfw_error_callback);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(viewWidth, viewHeight, "Shader Editor", nullptr, nullptr);

	if (window == nullptr) {
		std::cerr << "Failed to create GLFW window." << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cerr << "Failed to initialize GLAD" << std::endl;

		return EXIT_FAILURE;
	}

    // START SETUP
    //float vertices[] = {
    //    -0.5f, -0.5f, 0.0f,
    //     0.5f, -0.5f, 0.0f,
    //     0.0f,  0.5f, 0.0f
    //};
    float vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    // START SHADERS
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // END SHADERS

    // START BUFFERS
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Framebuffer
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Framebuffer color attachment
    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferSize.x, framebufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferSize.x, framebufferSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Framebuffer is incomplete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // END SETUP

	// Set up Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //ImFontConfig fontConfig;
    //fontConfig.SizePixels = 32.0f;
    //io.Fonts->AddFontDefault(&fontConfig);
    io.Fonts->AddFontFromFileTTF("JetBrainsMonoNerdFont-Regular.ttf", 32.0f);

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        style.Colors[ImGuiCol_FrameBg] = ImVec4{ 0.1, 0.1, 0.1, 1.0 };
        style.Colors[ImGuiCol_Button] = ImVec4{ 0.2, 0.2, 0.2, 1.0 };
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.15, 0.0, 0.25, 1.0 };
        style.Colors[ImGuiCol_TabActive] = ImVec4{ 0.15, 0.15, 0.15, 1.0 };
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.1, 0.1, 0.1, 1.0 };
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.0, 0.0, 0.0, 1.0 };
        style.FrameRounding = 5.0f;
        style.TabRounding = 5.0f;
        //ImGuiCol_Tab,
        //ImGuiCol_TabHovered,
        //ImGuiCol_TabActive,
        //ImGuiCol_TabUnfocused,
        //ImGuiCol_TabUnfocusedActive,
	}

	// Set up backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool show_demo_window = false;

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Dockspace
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);



        // Fragment shader window
        {

            static std::string fragmentShaderString = glsl_version + "\n\nin vec2 TexCoords;\nout vec4 FragColor;\n\nuniform vec2 ViewportSize;\n\nuniform float Time;\n\nvoid main()\n{\n\tFragColor = vec4(vec3(0), 1);\n}";

            ImGui::Begin("Fragment Shader");

            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
            ImGui::InputTextMultiline("User Input", &fragmentShaderString, ImVec2(-FLT_MIN, -ImGui::GetTextLineHeight() * 5), flags);

            if (ImGui::Button("Recompile")) {
                std::cout << "Recompiling..." << std::endl;

                vertexShader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
                glCompileShader(vertexShader);
                // check for shader compile errors

                glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
                }

                try {
                    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
                    const char* newFragmentShader = fragmentShaderString.c_str();
                    glShaderSource(fragmentShader, 1, &newFragmentShader, NULL);
                    glCompileShader(fragmentShader);
                    // check for shader compile errors
                    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
                    if (!success)
                    {
                        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
                    }
                    // link shaders
                    shaderProgram = glCreateProgram();
                    glAttachShader(shaderProgram, vertexShader);
                    glAttachShader(shaderProgram, fragmentShader);
                    glLinkProgram(shaderProgram);
                    // check for linking errors
                    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
                    if (!success) {
                        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
                    }
                    glDeleteShader(vertexShader);
                    glDeleteShader(fragmentShader);
                }
                catch (const std::exception& e) {
                    //useTempShaderProgram = false;
                    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
                    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
                    glCompileShader(fragmentShader);
                    // check for shader compile errors
                    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
                    if (!success)
                    {
                        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
                    }
                    // link shaders
                    shaderProgram = glCreateProgram();
                    glAttachShader(shaderProgram, vertexShader);
                    glAttachShader(shaderProgram, fragmentShader);
                    glLinkProgram(shaderProgram);
                    // check for linking errors
                    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
                    if (!success) {
                        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
                    }
                    glDeleteShader(vertexShader);
                    glDeleteShader(fragmentShader);
                }
            }

            ImGui::End();
        }

        {
            ImGui::Begin("Render Window");
            auto viewportSize = ImGui::GetContentRegionAvail();

            if (framebufferSize.x != viewportSize.x || framebufferSize.y != viewportSize.y) {
                framebufferSize = viewportSize;

                glDeleteFramebuffers(1, &framebuffer);
                glDeleteRenderbuffers(1, &rbo);

                glGenFramebuffers(1, &framebuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

                glViewport(0, 0, viewportSize.x, viewportSize.y);

                glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferSize.x, framebufferSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferSize.x, framebufferSize.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            }
            else {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
                glViewport(0, 0, viewportSize.x, viewportSize.y);
            }

            // Draw
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            glUseProgram(shaderProgram);
            int u_ViewportSize = glGetUniformLocation(shaderProgram, "ViewportSize");
            glUniform2f(u_ViewportSize, viewportSize.x, viewportSize.y);
            int u_Time = glGetUniformLocation(shaderProgram, "Time");
            glUniform1f(u_Time, glfwGetTime());

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            ImGui::Image((void*)textureColorBuffer, ImVec2(framebufferSize.x, framebufferSize.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::End();
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}