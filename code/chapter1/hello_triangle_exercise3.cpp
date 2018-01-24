//
//  hello_triangle_exercise1.cpp
//  hello_window
//
//  Created by 李文龙 on 2018/1/23.
//  Copyright © 2018年 liwenlong. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";
const char *fragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);\n"
"}\n\0";

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glewExperimental = GL_TRUE;
    glewInit();
    
    // shader compile
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char LogError[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, LogError);
        std::cout << "ERROR::VERTEXSHADER::COMPILTE::ERROR:" << LogError << std::endl;
    }
    
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, LogError);
        std::cout << "ERROR::FRAGMENTSHADER::COMPILTE::ERROR:" << LogError << std::endl;
    }
    
    int fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, NULL);
    glCompileShader(fragmentShader1);
    glGetShaderiv(fragmentShader1, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader1, 512, NULL, LogError);
        std::cout << "ERROR::FRAGMENTSHADER::COMPILTE::ERROR:" << LogError << std::endl;
    }
    
    // link shader
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, LogError);
        std::cout << "ERROR::PROGRAM::LINK::ERROR:" << LogError << std::endl;
    }
    
    int shaderProgram1 = glCreateProgram();
    glAttachShader(shaderProgram1, vertexShader);
    glAttachShader(shaderProgram1, fragmentShader1);
    glLinkProgram(shaderProgram1);
    
    glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram1, 512, NULL, LogError);
        std::cout << "ERROR::PROGRAM::LINK::ERROR:" << LogError << std::endl;
    }
    
    // free shader
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShader1);
    
    // data
    float vertices1[] = {
        0.5f, 0.5f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };
    
    float vertices2[] = {
        0.0f, 0.0f, 0.0f,
        -0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f
    };
    
    //unsigned int indices[] = {
    //    0, 1, 2,
    //    3, 4, 5
    //};
    
    unsigned int VAO, VBO;//EBO;
    unsigned int VAO1, VBO1;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    
    glBindVertexArray(VAO1);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glUseProgram(shaderProgram1);
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

