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
#include <algorithm>
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

        m_image = std::make_shared<Image>();

        std::string defaultVertSource =
        #include "render.vert.glsl"
        ;
        std::string defaultFragSource =
        #include "render.frag.glsl"
        ;
        const char* vertSource = defaultVertSource.c_str();
        const char* fragSource = defaultFragSource.c_str();
        GLuint vertID, fragID;
        vertID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertID, 1, &vertSource, nullptr);
        glCompileShader(vertID);
        GLint compileSuccess;
        glGetShaderiv(vertID, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(vertID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(vertID, infoLen, nullptr, &infoLog[0]);
            std::string content = std::string(&infoLog[0]);
            throw std::runtime_error("vertex shader failed to compile:\n"+content);
        }
        fragID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragID, 1, &fragSource, nullptr);
        glCompileShader(fragID);
        glGetShaderiv(fragID, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(fragID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(fragID, infoLen, nullptr, &infoLog[0]);
            std::string content = std::string(&infoLog[0]);
            throw std::runtime_error("fragment shader failed to compile:\n"+content);
        }
        m_render_shader = glCreateProgram();
        glAttachShader(m_render_shader, vertID);
        glAttachShader(m_render_shader, fragID);
        glLinkProgram(m_render_shader);
        GLint success;
        glGetProgramiv(m_render_shader, GL_LINK_STATUS, &success);
        if(!success)
        {
            GLint infoLen;
            glGetProgramiv(m_render_shader, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetProgramInfoLog(m_render_shader, infoLen, nullptr, &infoLog[0]);
            throw std::runtime_error("program failed to link: " + std::string(&infoLog[0]));
        }
        glDeleteShader(vertID);
        glDeleteShader(fragID);

        add_log_message("UI", "program initialized");
    }

    ~Application()
    {
        glDeleteProgram(m_render_shader);
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

            if(m_image->exists())
            {
                float ratioWin = winH ? (float)winW / (float)winH : 0.0f;
                int imgW = m_image->width();
                int imgH = m_image->height();
                float ratioImg = imgH ? (float)imgW / (float)imgH : 0.0f;
                GLuint lastImgOutput = m_image->source();
                for(size_t i = 0; i < m_shader_pipeline.size(); i++)
                {
                    auto& shader = m_shader_pipeline[i];
                    GLuint imgInput = lastImgOutput;
                    GLuint imgOutput = m_image->nextBuffer(i == 0);
                    lastImgOutput = imgOutput;
                    shader->bind();
                    glBindImageTexture(0, imgInput, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                    glBindImageTexture(1, imgOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                    glDispatchCompute((GLuint)imgW, (GLuint)imgH, 1);
                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    shader->unbind();
                }
                glUseProgram(m_render_shader);
                glUniform1f(glGetUniformLocation(m_render_shader, "ratio_img"), ratioImg);
                glUniform1f(glGetUniformLocation(m_render_shader, "ratio_win"), ratioWin);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, lastImgOutput);
                glUniform1i(glGetUniformLocation(m_render_shader, "image"), 0);
                m_image->draw();
                glUseProgram(0);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            

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
                        if(ImGui::Button("Set"))
                        {
                            if(!m_image->update(tools_select_file("Image", "*.*")))
                                add_log_message("Image", m_image->get_error());
                            else add_log_message("UI", "image file updated");
                        }
                        ImGui::Separator();
                        ImGui::Text("Shader Pipeline Layers");
                        auto iter = m_shader_pipeline.begin();
                        int i = 0;
                        while(iter != m_shader_pipeline.end())
                        {
                            ImGui::PushID(i);
                            auto& shader = *iter;
                            ImGui::Text("[%d] %s", i, shader->get_name().c_str());
                            if(ImGui::Button("Del"))
                            {
                                iter = m_shader_pipeline.erase(iter);
                                i++;
                                ImGui::PopID();
                                continue;
                            }
                            if(i > 0)
                            {
                                ImGui::SameLine();
                                if(ImGui::Button("up"))
                                    std::iter_swap(iter, iter-1);
                            }
                            if(i < (int)m_shader_pipeline.size() - 1)
                            {
                                ImGui::SameLine();
                                if(ImGui::Button("down"))
                                    std::iter_swap(iter, iter+1);
                            }
                            i++;
                            iter++;
                            ImGui::PopID();
                        }
                        if(ImGui::Button("Add"))
                        {
                            std::shared_ptr<Shader> newshader = std::make_shared<Shader>(
                                tools_select_file("Shader", "*.glsl")
                            );
                            if(!newshader->initialized())
                                add_log_message("Shader", newshader->get_error());
                            else
                            {
                                m_shader_pipeline.push_back(newshader);
                                add_log_message("UI", newshader->get_name() + " added");
                            }
                        }
                        ImGui::Text("[Final] Output");
                        ImGui::EndTabItem();
                    }
                    if(ImGui::BeginTabItem("Log"))
                    {
                        for(auto& message : m_log)
                        {
                            ImGui::TextWrapped("%s", message.c_str());
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
                    for(auto& shader : user->m_shader_pipeline)
                    {
                        if(!shader->update())
                            user->add_log_message("Shader", shader->get_error());
                    }
                    user->add_log_message("Shader", "shader pipeline refreshed");
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
    GLuint m_render_shader;
    std::vector<std::shared_ptr<Shader>> m_shader_pipeline;
    std::shared_ptr<Image> m_image;
};
