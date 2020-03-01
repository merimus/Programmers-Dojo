#pragma once

#include "mandelbrot.h"
#include "shader.h"

class computeGPU : public computeBase {
private:
    int computeProgram;
public:
    computeGPU(Mandelbrot& m) : computeBase(m) {
        Shader shader;
        int computeShader = shader.newShader(GL_COMPUTE_SHADER, "mandelbrot.glsl");
        computeProgram = glCreateProgram();
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);
        // check for linking errors
        int success;
        glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(computeShader);
    }

    void compute() {
        auto start = high_resolution_clock::now();
        
        glBindTexture(GL_TEXTURE_2D, mb.tex_output);
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "cxmin"), mb.cxmin.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cxmax"), mb.cxmax.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cymin"), mb.cymin.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cymax"), mb.cymax.convert_to<float>());
        glUniform1i(glGetUniformLocation(computeProgram, "resolution"), mb.resolution);
        glUniform1i(glGetUniformLocation(computeProgram, "max_iterations"), mb.max_iterations);

        glDispatchCompute(2048 / mb.resolution / 16, 2048 / mb.resolution / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        frame_time_ = duration_cast<microseconds>(high_resolution_clock::now() - start);
    }
};
