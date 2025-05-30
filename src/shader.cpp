/*
 * Copyright (C) 2023  Renan S. Silva, aka h3nnn4n
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <sys/inotify.h>

#include <errno.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "glsl_shader_includes.hpp"
#include "shader.hpp"

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

void check_inotify_errors(int return_value) {
    if (return_value < 0) {
        std::cerr << "Error watching file" << std::endl;
    }
}

Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
    useGeometryShader = geometryPath != NULL;

    this->vertexPath.assign(vertexPath);
    this->fragmentPath.assign(fragmentPath);

    if (useGeometryShader)
        this->geometryPath.assign(geometryPath);

    std::cout << "Loading " << vertexPath << " - " << fragmentPath;

    if (useGeometryShader)
        std::cout << " - " << geometryPath << std::endl;

    std::cout << std::endl;

    inotify_fd = inotify_init1(IN_NONBLOCK);

    if (inotify_fd < 0) {
        std::cerr << "Error initializing inotify" << std::endl;
    } else {
        check_inotify_errors(inotify_add_watch(inotify_fd, vertexPath, IN_MODIFY));
        check_inotify_errors(inotify_add_watch(inotify_fd, fragmentPath, IN_MODIFY));
        check_inotify_errors(inotify_add_watch(inotify_fd, "shaders/", IN_MODIFY));

        if (useGeometryShader)
            check_inotify_errors(inotify_add_watch(inotify_fd, geometryPath, IN_MODIFY));
    }

    load(vertexPath, fragmentPath, geometryPath);
}

void Shader::reload_changes() {
    char buffer[EVENT_BUF_LEN];

    int length = read(inotify_fd, buffer, EVENT_BUF_LEN);

    if (length < 0 && errno != EAGAIN) {
        perror("read");
    }

    if (length > 0) {
        std::cerr << "hot reloading shaders" << std::endl;
        load(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());
    }
}

void Shader::load(const char *vertexPath, const char *fragmentPath, const char *geometryPath) {
    Shadinclude preprocessor = Shadinclude();

    std::string vertexCode   = preprocessor.load(vertexPath);
    std::string fragmentCode = preprocessor.load(fragmentPath);
    std::string geometryCode;

    if (useGeometryShader)
        geometryCode = preprocessor.load(geometryPath);

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    const char *gShaderCode = NULL;

    if (useGeometryShader)
        gShaderCode = geometryCode.c_str();

    unsigned int vertex;
    unsigned int fragment;
    unsigned int geometry;

    {
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX", vertexPath);
    }

    {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT", fragmentPath);
    }

    // geometry Shader
    if (useGeometryShader) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY", geometryPath);
    }

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    if (useGeometryShader)
        glAttachShader(ID, geometry);

    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (useGeometryShader)
        glDeleteShader(geometry);
}

void Shader::use() { glUseProgram(ID); }

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string &name, float *value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float v1, float v2, float v3) const {
    unsigned int uniformLoc = glGetUniformLocation(ID, name.c_str());
    glUniform3f(uniformLoc, v1, v2, v3);
}

void Shader::setMatrix4(const std::string &name, float *value) const {
    unsigned int uniformLoc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, value);
}

void Shader::checkCompileErrors(GLuint shader, std::string type, std::string extra) {
    GLint  success;
    GLchar infoLog[2048];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(shader, 2048, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << " FILE: " << extra << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(shader, 2048, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
