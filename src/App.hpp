#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Shader.hpp"
#include "Image.hpp"
#include "Tools.hpp"

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

#define MAX_LOG_MESSAGE 100

class Application
{
public:
    Application() : winTitle("Image Processing"), m_log()
    {
        winW = 800;
        winH = 600;
        if(!glfwInit())
            throw std::runtime_error("GLFW: Failed to init");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        m_window = glfwCreateWindow(winW, winH, winTitle, nullptr, nullptr);
        if(!m_window)
            throw std::runtime_error("GLFW: Failed to create window");
        glfwSetWindowUserPointer(m_window, this);
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);
        glfwSetKeyCallback(m_window, glfw_key_callback);
        
        glewExperimental = GL_TRUE;
        if(glewInit() != GLEW_OK)
            throw std::runtime_error("GLEW: Failed to init");
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        m_shader = std::make_shared<Shader>();
        m_image = std::make_shared<Image>();
        add_log_message("UI", "program initialized");
    }

    ~Application()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void run()
    {
        while(!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            glfwGetFramebufferSize(m_window, &winW, &winH);
            glViewport(0, 0, winW, winH);
            glClearColor(winBackground[0], winBackground[1], winBackground[2], winBackground[3]);
            glClear(GL_COLOR_BUFFER_BIT);

            float ratio = (float)winW / (float)winH;
            m_shader->bind();
            m_shader->setUniform1f("ratio_img", m_image->get_ratio());
            m_shader->setUniform1f("ratio_win", ratio);
            m_image->render(*m_shader);
            m_shader->unbind();

            if(displayUI)
            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGui::SetNextWindowSize({400.0f, 300.0f}, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
                ImGui::Begin("UI");
                if(ImGui::BeginTabBar("Configs"))
                {
                    if(ImGui::BeginTabItem("App"))
                    {
                        ImGui::Text("Window Size: %dx%d", winW, winH);
                        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
                        ImGui::ColorEdit4("Background Color", winBackground);
                        if(ImGui::Checkbox("OpenGL Debug", &debugMode)) updateGLDebug();
                        ImGui::Text("Author: Teamclouday");
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Image Processing"))
                    {
                        ImGui::Text("Image file: %s", m_image->exists() ? m_image->get_name().c_str() : "none");
                        if(ImGui::Button("Select Image"))
                        {
                            if(!m_image->update(tools_select_file("Image", "*.*")))
                                add_log_message("Image", m_image->get_error());
                            else add_log_message("UI", "image file updated");
                        }
                        auto shaderName = m_shader->get_name();
                        ImGui::Text("Shader file: %s", shaderName.length() > 0 ? shaderName.c_str() : "default");
                        if(ImGui::Button("Select Shader"))
                        {
                            if(!m_shader->update(tools_select_file("Shader", "*.glsl")))
                                add_log_message("Shader", m_shader->get_error());
                            else add_log_message("UI", "shader file updated");
                        }
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Log"))
                    {
                        for(auto& message : m_log)
                        {
                            ImGui::Text(message.c_str());
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::End();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            glfwSwapBuffers(m_window);
        }
    }

    void updateGLDebug()
    {
        if(debugMode)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(GLDebugCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
        else
        {
            glDisable(GL_DEBUG_OUTPUT);
            glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        }
    }

private:
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Application* user = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if(action == GLFW_PRESS)
        {
            switch(key)
            {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                case GLFW_KEY_R:
                    if(!user->m_shader->update())
                        user->add_log_message("Shader", user->m_shader->get_error());
                    else user->add_log_message("UI", "shader file refreshed");
                    break;
                case GLFW_KEY_F12:
                    user->displayUI = !user->displayUI;
                    break;
            }
        }
    }

    void add_log_message(const std::string& title, const std::string& message)
    {
        std::string final_message = "[" + title + "] " + message;
        m_log.insert(m_log.begin(), final_message);
        while(m_log.size() > MAX_LOG_MESSAGE) m_log.pop_back();
    }


public:
    const char* winTitle;
    int winW, winH;
    float winBackground[4] = {0.4f,0.4f,0.4f,1.0f};
    bool debugMode = false;
    bool displayUI = true;

private:
    GLFWwindow* m_window;
    std::vector<std::string> m_log;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Image> m_image;
};
