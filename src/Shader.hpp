#pragma once

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct ShaderControl
{
    std::string name;
    bool isInt;
    union
    {
        int val_int;
        float val_float;
    } val;
};

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
        file.close();
        size_t newHash = hashing(shaderSourceStr);
        if(newHash == sourceHash) return true;
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
        sourceHash = newHash;
        processSource(shaderSourceStr);
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

    std::vector<ShaderControl> controls;

private:
    GLuint m_program;
    bool m_initialized = false;
    std::string error_message = "";
    std::string m_filepath = "";
    std::string m_name = "";
    std::hash<std::string> hashing;
    size_t sourceHash = 0;

    void processSource(const std::string& source)
    {
        controls.resize(0);
        size_t linestart = 0;
        size_t linesplit = source.find("\n");
        while(linesplit != std::string::npos)
        {
            auto line = source.substr(linestart, linesplit - linestart);
            if(line.find("uniform") == 0)
            {
                // found uniform
                line = line.substr(7);
                std::string type;
                std::string name;
                std::string value;
                auto charIter = line.cbegin();
                while(charIter != line.cend())
                {
                    if(*charIter == ' ') charIter++;
                    else break;
                }
                while(charIter != line.cend())
                {
                    if(*charIter == ' ') break;
                    type += *charIter;
                    charIter++;
                }
                while(charIter != line.cend())
                {
                    if(*charIter == ' ') charIter++;
                    else break;
                }
                while(charIter != line.cend())
                {
                    if(*charIter == ';' || *charIter == ' ') break;
                    name += *charIter;
                    charIter++;
                }
                while(charIter != line.cend())
                {
                    if(*charIter == ' ' || *charIter == '=') charIter++;
                    else break;
                }
                while(charIter != line.cend())
                {
                    if(*charIter == ';' || *charIter == ' ') break;
                    value += *charIter;
                    charIter++;
                }
                if(name.length() > 0)
                {
                    if(!type.compare("int"))
                    {
                        ShaderControl control;
                        control.name = name;
                        control.isInt = true;
                        control.val.val_int = value.length() > 0 ? std::stoi(value) : 0;
                        controls.push_back(control);
                    }
                    else if(!type.compare("float"))
                    {
                        ShaderControl control;
                        control.name = name;
                        control.isInt = false;
                        control.val.val_float = value.length() > 0 ? std::stof(value) : 0.0f;
                        controls.push_back(control);
                    }
                }
            }
            linestart = linesplit + 1;
            linesplit = source.find("\n", linestart);
        }

    }
};
