#pragma once

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class Shader
{
public:
    Shader(const std::string& filepath)
    {
        m_filepath = filepath;
        m_name = filepath.substr(filepath.find_last_of("/\\") + 1);
        m_initialized = update();
    }

    ~Shader()
    {
        if(m_initialized) glDeleteProgram(m_program);
    }

    bool update()
    {
        std::ifstream file(m_filepath);
        if(!file.is_open())
        {
            error_message = "failed to read source file: " + m_filepath;
            return false;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string shaderSourceStr = ss.str();
        const char* shaderSource = shaderSourceStr.c_str();
        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(shader, 1, &shaderSource, nullptr);
        glCompileShader(shader);
        GLint compileSuccess;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetShaderInfoLog(shader, infoLen, nullptr, &infoLog[0]);
            error_message = "compute shader failed to compile:\n"+ std::string(&infoLog[0]);
            glDeleteShader(shader);
            return false;
        }
        GLuint newprogram = glCreateProgram();
        glAttachShader(newprogram, shader);
        glLinkProgram(newprogram);
        glGetProgramiv(newprogram, GL_LINK_STATUS, &compileSuccess);
        if(!compileSuccess)
        {
            GLint infoLen;
            glGetProgramiv(newprogram, GL_INFO_LOG_LENGTH, &infoLen);
            std::vector<GLchar> infoLog(infoLen+1);
            glGetProgramInfoLog(newprogram, infoLen, nullptr, &infoLog[0]);
            error_message = "compute shader failed to link:\n"+ std::string(&infoLog[0]);
            glDeleteShader(shader);
            glDeleteProgram(newprogram);
            return false;
        }
        if(m_initialized) glDeleteProgram(m_program);
        m_program = newprogram;
        glDeleteShader(shader);
        return true;
    }

    std::string get_error() const
    {
        return error_message;
    }

    std::string get_name() const
    {
        return m_name;
    }

    void bind() const
    {
        if(m_initialized) glUseProgram(m_program);
    }

    void unbind() const
    {
        glUseProgram(0);
    }

    bool initialized() const
    {
        return m_initialized;
    }

    void setUniform1i(const std::string& name, int val) const
    {
        if(!m_initialized) return;
        glUniform1i(glGetUniformLocation(m_program, name.c_str()), val);
    }

    void setUniform1f(const std::string& name, float val) const
    {
        if(!m_initialized) return;
        glUniform1f(glGetUniformLocation(m_program, name.c_str()), val);
    }

private:
    GLuint m_program;
    bool m_initialized = false;
    std::string error_message = "";
    std::string m_filepath = "";
    std::string m_name = "";
};
