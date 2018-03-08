//
//  shader_m.h
//  hello_window
//
//  Created by 李文龙 on 2018/2/11.
//  Copyright © 2018年 liwenlong. All rights reserved.
//

#ifndef shader_m_h
#define shader_m_h

#include <GL/glew.h>
#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID;
    
    Shader(char const *vertexPath, char const* fragmentPath) {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exception
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        char const* vShaderCode = vertexCode.c_str();
        char const* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    void use() const {
        glUseProgram(ID);
    }
    
    void setBool(std::string const& name_, bool value_) const {
        glUniform1i(glGetUniformLocation(ID, name_.c_str()), (int)value_);
    }
    
    void setInt(std::string const& name_, int value_) const {
        glUniform1i(glGetUniformLocation(ID, name_.c_str()), value_);
    }
    
    void setFloat(std::string const& name_, float value_) const {
        glUniform1f(glGetUniformLocation(ID, name_.c_str()), value_);
    }
    
    void setVec2(std::string const& name_, glm::vec2 const& value_) const {
        glUniform2fv(glGetUniformLocation(ID, name_.c_str()), 1, &value_[0]);
    }
    
    void setVec2(std::string const& name_, float x, float y) const {
        glUniform2f(glGetUniformLocation(ID, name_.c_str()), x, y);
    }
    
    void setVec3(std::string const& name_, glm::vec3 const& value_) const {
        glUniform3fv(glGetUniformLocation(ID, name_.c_str()), 1, &value_[0]);
    }
    
    void setVec3(std::string const& name_, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name_.c_str()), x, y, z);
    }
    
    void setVec4(std::string const& name_, glm::vec4 const& value_) const {
        glUniform4fv(glGetUniformLocation(ID, name_.c_str()), 1, &value_[0]);
    }
    
    void setVec4(std::string const& name_, float x, float y, float z, float w) {
        glUniform4f(glGetUniformLocation(ID, name_.c_str()), x, y, z, w);
    }
    
    void setMat2(std::string const& name_, glm::mat2 const& mat) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name_.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat3(std::string const& name_, glm::mat3 const& mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name_.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setMat4(std::string const& name_, glm::mat4 const& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name_.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n-----------------" << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n-----------------" << std::endl;
            }
        }
    }
};


#endif /* shader_m_h */
