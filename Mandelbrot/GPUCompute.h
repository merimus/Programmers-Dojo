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
        
        unsigned int gradient;
        glGenTextures(1, &gradient);
        glBindTexture(GL_TEXTURE_1D, gradient);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 1024, 1, 0, GL_UNSIGNED_BYTE, mb.gradient);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glBindTexture(GL_TEXTURE_2D, mb.tex_output);
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "cxmin"), mb.cxmin.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cxmax"), mb.cxmax.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cymin"), mb.cymin.convert_to<float>());
        glUniform1f(glGetUniformLocation(computeProgram, "cymax"), mb.cymax.convert_to<float>());
        glUniform1i(glGetUniformLocation(computeProgram, "gradient"), gradient);

        glDispatchCompute(2048 / 16, 2048 / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        frame_time_ = duration_cast<microseconds>(high_resolution_clock::now() - start);

        glDeleteTextures(1, &gradient);
    }
};
