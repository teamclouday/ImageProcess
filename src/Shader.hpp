#pragma once

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

struct ShaderComponent
{
    GLuint ID;
    bool compiled = false;
};

class Shader
{
public:
    Shader()
    {
        std::string defaultVertSource =
        #include "render.vert.glsl"
        ;
        std::string defaultFragSource =
        #include "render.frag.glsl"
        ;
        const char* vertSource = defaultVertSource.c_str();
        const char* fragSource = defaultFragSource.c_str();
        m_vert_shader.ID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vert_shader.ID, 1, &vertSource, nullptr);
        glCompileShader(m_vert_shader.ID);
        GLint compileSuccess;
        glGetShaderiv(m_vert_shader.ID, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(m_vert_shader.ID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(m_vert_shader.ID, infoLen, nullptr, &infoLog[0]);
            std::string content = std::string(&infoLog[0]);
            throw std::runtime_error("vertex shader failed to compile:\n"+content);
        }
        m_vert_shader.compiled = true;
        m_frag_shader.ID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_frag_shader.ID, 1, &fragSource, nullptr);
        glCompileShader(m_frag_shader.ID);
        glGetShaderiv(m_frag_shader.ID, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(m_frag_shader.ID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(m_frag_shader.ID, infoLen, nullptr, &infoLog[0]);
            std::string content = std::string(&infoLog[0]);
            throw std::runtime_error("fragment shader failed to compile:\n"+content);
        }
        m_frag_shader.compiled = true;
        if(!link())
            throw std::runtime_error(error_message);
        m_frag_shader.compiled = true;
    }

    ~Shader()
    {
        if(m_program.compiled) glDeleteProgram(m_program.ID);
        if(m_vert_shader.compiled) glDeleteShader(m_vert_shader.ID);
        if(m_frag_shader.compiled) glDeleteShader(m_frag_shader.ID);
    }

    bool update(const std::string& filename)
    {
        std::ifstream file(filename);
        if(!file.is_open())
        {
            error_message = "failed to read source file: " + filename;
            return false;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string fragShaderSource = ss.str();
        const char* fragShader = fragShaderSource.c_str();
        std::cout << fragShaderSource << std::endl;
        if(m_frag_shader.compiled)
        {
            glDeleteShader(m_frag_shader.ID);
            m_frag_shader.compiled = false;
        }
        m_frag_shader.ID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_frag_shader.ID, 1, &fragShader, nullptr);
        glCompileShader(m_frag_shader.ID);
        GLint compileSuccess;
        glGetShaderiv(m_frag_shader.ID, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(m_frag_shader.ID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(m_frag_shader.ID, infoLen, nullptr, &infoLog[0]);
            error_message = "fragment shader failed to compile:\n"+ std::string(&infoLog[0]);
            return false;
        }
        m_frag_shader.compiled = true;
        m_frag_path = filename;
        return link();
    }

    bool update()
    {
        if(m_frag_path.length()) return update(m_frag_path);
        return true;
    }

    std::string get_error() const
    {
        return error_message;
    }

    std::string get_name() const
    {
        return m_frag_path;
    }

    void bind() const
    {
        if(m_program.compiled) glUseProgram(m_program.ID);
    }

    void unbind() const
    {
        glUseProgram(0);
    }

    void setUniform1i(const std::string& name, int val) const
    {
        if(!m_program.compiled) return;
        glUniform1i(glGetUniformLocation(m_program.ID, name.c_str()), val);
    }

    void setUniform1f(const std::string& name, float val) const
    {
        if(!m_program.compiled) return;
        glUniform1f(glGetUniformLocation(m_program.ID, name.c_str()), val);
    }

private:
    ShaderComponent m_program;
    ShaderComponent m_vert_shader;
    ShaderComponent m_frag_shader;
    std::string error_message = "";
    std::string m_frag_path = "";

    bool link()
    {
        if(!m_vert_shader.compiled || !m_frag_shader.compiled) return false;
        if(m_program.compiled)
        {
            glDeleteProgram(m_program.ID);
            m_program.compiled = false;
        }
        m_program.ID = glCreateProgram();
        glAttachShader(m_program.ID, m_vert_shader.ID);
        glAttachShader(m_program.ID, m_frag_shader.ID);
        glLinkProgram(m_program.ID);
        GLint success;
        glGetProgramiv(m_program.ID, GL_LINK_STATUS, &success);
        if(!success)
        {
            GLint infoLen;
            glGetProgramiv(m_program.ID, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetProgramInfoLog(m_program.ID, infoLen, nullptr, &infoLog[0]);
            error_message = "program failed to link: " + std::string(&infoLog[0]);
            return false;
        }
        m_program.compiled = true;
        return true;
    }
};
