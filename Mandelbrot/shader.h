#pragma once

#include <iostream>
#include <fstream>
#include <GL/glew.h>

class Shader {
  public:
    Shader() {}
    virtual ~Shader() {}
    
    unsigned int newShader(GLenum t, const std::string& f) {
        std::fstream inFile(f, std::ios::in);
        if(!inFile.is_open()) {
            std::cerr << "Failed to open file " << f << std::endl;
            return 0;
        }
        std::stringstream source;
        source << inFile.rdbuf();
        unsigned int shaderId;
        shaderId = glCreateShader(t);

        char* tmp = strdup(source.str().c_str());
        glShaderSource(shaderId, 1, &tmp, NULL);
        free(tmp);
        glCompileShader(shaderId);
    
        int  success;
        char infoLog[512];
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
          glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
          std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        return shaderId;
    }
};
