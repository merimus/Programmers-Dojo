#pragma once

#include "mandelbrot.h"

template <typename T> class computeCPU : public computeBase {
private:
    uint8_t *tex_data;

public:
    computeCPU(Mandelbrot &m) : computeBase(m) {
        tex_data = (uint8_t*)malloc(sizeof(uint8_t) * 2048 * 2048 * 4);
    }

    void compute() {
        auto start = high_resolution_clock::now();
        double cxmin = mb.cxmin.convert_to<double>();
        double cxmax = mb.cxmax.convert_to<double>();
        double cymin = mb.cymin.convert_to<double>();
        double cymax = mb.cymax.convert_to<double>();

        double xdelta = cxmax - cxmin;
        double ydelta = cymax - cymin;
        int width = 2048 / mb.resolution;

        #pragma omp parallel for
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < width; ++y) {
                int idx = 4 * (y + x * 2048); 
                std::complex<T> c(cxmin + (y / (T)width * xdelta),
                                  cymin + (x / (T)width * ydelta));
                std::complex<T> z;

                int max_iterations = mb.max_iterations;
                int iterations;
                for (iterations = 0; iterations < max_iterations && abs(z) < 2.0; ++iterations) {
                    z = z * z + c;
                }

                if (iterations < max_iterations) {
                    double esc = iterations;
                    esc += 1 - log(log2(abs(z)));
                    double q = esc / max_iterations;

                    memcpy(&tex_data[idx], &mb.gradient[(int)(q * 1023) * 4], sizeof(float) * 4);
                } else {
                    tex_data[idx + 0] = 0;
                    tex_data[idx + 1] = 0;
                    tex_data[idx + 2] = 0;
                    tex_data[idx + 3] = 0;
                }
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
        frame_time_ = duration_cast<microseconds>(high_resolution_clock::now() - start);
    }
};
