#pragma once

#include <GL/glew.h>
#include <stb_image.h>

#include "Shader.hpp"

#include <string>

class Image
{
public:
    Image()
    {
        const float vertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f,
        };
        GLuint VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    ~Image()
    {
        glDeleteVertexArrays(1, &VAO);
        if(m_image_exist) glDeleteTextures(1, &m_image);
    }

    void render(const Shader& shader) const
    {
        if(!m_image_exist) return;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_image);
        shader.setUniform1i("image", 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    bool update(const std::string& filepath)
    {
        if(m_image_exist)
        {
            glDeleteTextures(1, &m_image);
            m_image_exist = false;
        }
        stbi_set_flip_vertically_on_load(true);
        unsigned char* img = stbi_load(filepath.c_str(), &m_imageW, &m_imageH, &m_imageC, 0);
        if(!img)
        {
            error_message = "failed to load image file: " + filepath;
            return false;
        }
        glGenTextures(1, &m_image);
        glBindTexture(GL_TEXTURE_2D, m_image);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLenum imgFormat = GL_RGBA;
        switch (m_imageC)
        {
        case 1:
            imgFormat = GL_RED;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            break;
        case 2:
            imgFormat = GL_RG;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
            break;
        case 3:
            imgFormat = GL_RGB;
            break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_imageW, m_imageH, 0, imgFormat, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(img);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_image_exist = true;
        m_image_path = filepath;
        return m_image_exist;
    }

    std::string get_error() const
    {
        return error_message;
    }

    std::string get_name() const
    {
        return m_image_path;
    }

    float get_ratio() const
    {
        if(!m_imageH) return 0.0f;
        return (float)m_imageW / (float)m_imageH;
    }

    bool exists() const
    {
        return m_image_exist;
    }

private:
    int m_imageW, m_imageH, m_imageC;
    GLuint m_image;
    bool m_image_exist = false;
    GLuint VAO;
    std::string error_message = "";
    std::string m_image_path = "";
};
