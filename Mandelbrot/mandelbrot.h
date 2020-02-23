#pragma once

#include <chrono>
using namespace std::chrono;
#include <GL/glew.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
namespace mp = boost::multiprecision;
typedef mp::number<mp::cpp_dec_float<0>> bigfloat;

class Mandelbrot {
public:
    bigfloat cxmin = -1.5;
    bigfloat cxmax = 0.5;
    bigfloat cymin = -1.0;
    bigfloat cymax = 1.0;
    bool dirty = true;
    int resolution = 0;
    int max_iterations = 50;
    std::string backend;
    GLuint tex_output;
    uint8_t gradient[1024 * 4];

    Mandelbrot(GLuint tex_output) : tex_output(tex_output) {}
    void Zoom(int z) {
        auto xdiff = z * (cxmax - cxmin) * 0.01;
        auto ydiff = z * (cymax - cymin) * 0.01;
        cxmin += float(xdiff);
        cxmax -= float(xdiff);
        cymin += float(ydiff);
        cymax -= float(ydiff);
        dirty = true;
    }

    void Move(double xd, double yd) {
        auto xstep = (cxmax - cxmin) * xd;
        auto ystep = (cymax - cymin) * yd;
        cxmin += xstep;
        cxmax += xstep;
        cymin -= ystep;
        cymax -= ystep;
        dirty = true;
    }
};

class computeBase {
protected:
    Mandelbrot &mb;
    microseconds frame_time_;

public:
    computeBase(Mandelbrot& mb) : mb(mb) {}
    microseconds frame_time() {return frame_time_; }
    virtual void compute() = 0;
};

