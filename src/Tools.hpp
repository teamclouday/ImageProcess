#pragma once

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#include <commdlg.h>
#else
#include <unistd.h>
#endif

#include <GL/glew.h>

#include <string>
#include <iostream>

std::string tools_select_file(const std::string& name, const std::string& extension)
{
    char filepath[1024];
#if defined(WIN32) || defined(_WIN32)
    filepath[0] = '\0';
    OPENFILENAMEA f{};
    f.lStructSize = sizeof(OPENFILENAMEA);
    f.hwndOwner = NULL;
    f.lpstrFile = filepath;
    std::string filter = name + " (" + extension + ")";
    filter.push_back('\0');
    filter.append(extension);
    filter.push_back('\0');
    filter.append("All Files (*.*)");
    filter.push_back('\0');
    filter.append("*.*");
    filter.push_back('\0');
    f.lpstrFilter = filter.c_str();
    f.lpstrTitle = "Select File";
    f.nMaxFile = 1024;
    f.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
    if(!GetOpenFileNameA(&f)) return "";
#else
    std::string command = "zenity --file-selection --title=\"Select File\" --file-filter='" + name + " | " + extension + "'";
    FILE* f = popen(command.c_str(), "r");
    fgets(filepath, 1023, f);
    filepath[strlen(filepath) - 1] = 0;
    pclose(f);
#endif
    return std::string(filepath);
}

void GLAPIENTRY GLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam
)
{
    std::cout << "**** GL Callback ****" << std::endl;
    std::cout << "(";
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:            std::cout << "High"; break;
        case GL_DEBUG_SEVERITY_LOW:             std::cout << "Low"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:          std::cout << "Med"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:    std::cout << "Noti"; break;
    }
    std::cout << ") ";
    std::cout << "<";
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:               std::cout << "API"; break;
        case GL_DEBUG_SOURCE_APPLICATION:       std::cout << "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:             std::cout << "Other"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   std::cout << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:       std::cout << "Third Party"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     std::cout << "Window System"; break;
    }
    std::cout << "> ";
    std::cout << "[";
    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Other"; break;
    }
    std::cout << "] " << std::endl;
    std::cout << message << std::endl;
}
