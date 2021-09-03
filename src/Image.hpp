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

    bool update(const std::string& filepath)
    {
        if(m_image_exist)
        {
            glDeleteTextures(1, &m_image);
            glDeleteTextures(2, m_image_buffer);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_imageW, m_imageH, 0, imgFormat, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(img);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(2, m_image_buffer);
        for(int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, m_image_buffer[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_imageW, m_imageH);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        m_image_exist = true;
        m_name = filepath.substr(filepath.find_last_of("/\\") + 1);
        return m_image_exist;
    }

    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    std::string get_error() const
    {
        return error_message;
    }

    std::string get_name() const
    {
        return m_name;
    }

    int width() const
    {
        if(!m_image_exist) return 0;
        return m_imageW;
    }

    int height() const
    {
        if(!m_image_exist) return 0;
        return m_imageH;
    }

    bool exists() const
    {
        return m_image_exist;
    }

    GLuint source() const
    {
        return m_image;
    }

    GLuint nextBuffer(bool reset = false) const
    {
        static int idx = 0;
        if(reset) idx = 0;
        GLuint buffer = m_image_buffer[idx];
        idx = !idx;
        return buffer;
    }

private:
    int m_imageW, m_imageH, m_imageC;
    GLuint m_image;
    GLuint m_image_buffer[2];
    bool m_image_exist = false;
    GLuint VAO;
    std::string error_message = "";
    std::string m_name = "";
};
