#include <iostream>
#include <fstream>
#include <chrono>
using namespace std::chrono;
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mandelbrot.h"
#include "shader.h"
#include "cpuCompute.h"
#include "GPUCompute.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

void make_gradient(uint8_t base[4], uint8_t gradient[1024*4]) {
    for (int i = 0; i < 1024; ++i) {
        int idx = i * 4;
        gradient[idx+0] = base[0] * i / 1023;
        gradient[idx+1] = base[1] * i / 1023;
        gradient[idx+2] = base[2] * i / 1023;
        gradient[idx+3] = base[3] * i / 1023;
    }
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void scroll_callback(GLFWwindow* window, double xo, double yo);

class Window {
public:
    GLFWwindow *window;
    int shaderProgram;
    GLuint tex_output;
    int resolution = 4;
    int button = 0;
    double scroll_x = 0;
    double scroll_y = 0;
    double x, y;
    int max_iterations = 50;
    std::vector<std::string> compute_backends;
    int selected_backend;
    float old_color[3];
    float color[3] = {0, 0, 1};
    microseconds last_frame_time;

    Window() {
        if (!glfwInit()) {
            exit(EXIT_FAILURE);
        }

        window = glfwCreateWindow(1024, 1024, "Mandelbrot", NULL, NULL);
        if (!window)
         {
            std::cerr << "Failed to create window" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(window);
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        ImGui::StyleColorsDark();

        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            std::cerr << "Error at glewInit " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
        }

        std::cout << "Glew " << glewGetString(GLEW_VERSION) << std::endl;
        std::cout << "Renderer: " <<  glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

        glfwSetFramebufferSizeCallback(window, resize_callback);
        glfwSetScrollCallback(window, scroll_callback);


        Shader shader;
        int vertexShader = shader.newShader(GL_VERTEX_SHADER, "vertex.glsl");
        int fragmentShader = shader.newShader(GL_FRAGMENT_SHADER, "fragment.glsl");

        int success;
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);

        glBindAttribLocation(shaderProgram, 0, "myVertex");
        glBindAttribLocation(shaderProgram, 1, "myUV");

        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);


        const int tex_w = 2048, tex_h = 2048;

        glGenTextures(1, &tex_output);
        glBindTexture(GL_TEXTURE_2D, tex_output);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    }

    ~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void scroll(double xo, double yo) {
        scroll_x += xo;
        scroll_y += yo;
    }

    void handleInput(Mandelbrot &mandelbrot) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        int s = (int)scroll_y;
        if (s != 0) {
            mandelbrot.Zoom(s);
            scroll_y = 0;
        }

        if (!ImGui::GetIO().WantCaptureMouse
            && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            if (button == 0) {
                button = 1;
                glfwGetCursorPos(window, &x, &y);
            } else {
                double new_x, new_y;
                glfwGetCursorPos(window, &new_x, &new_y);
                if (x != new_x || y != new_y) {
                    int width, height;
                    glfwGetWindowSize(window, &width, &height);
                    mandelbrot.Move((x - new_x) / width, (y - new_y) / height);
                    x = new_x;
                    y = new_y;
                }
            }
        } else {
            button = 0;
        }
    }

    void draw() {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glUseProgram(shaderProgram);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glBindTexture(GL_TEXTURE_2D, tex_output);
        
        GLuint VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        float s = 1.0f;
        float t = 1.0f;
        if (height > width) {
            s = (float)width / height;
        } else {
            t = (float)height / width;
        }
        glGenBuffers(1, &VBO);

        float smin = 0.5f / resolution - (s / 2.0f / resolution);
        float smax = 0.5f / resolution + (s / 2.0f / resolution);
        float tmin = 0.5f / resolution - (t / 2.0f / resolution);
        float tmax = 0.5f / resolution + (t / 2.0f / resolution);
        /* x, y, z, s, t */
        float vertices[] = {
            -1.0f,  1.0f, 0.f, smin, tmax,
            -1.0f, -1.0f, 0.f, smin, tmin,
            1.0f,  1.0f, 0.f, smax, tmax,
            1.0f, -1.0f, 0.f, smax, tmin,
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
            (void*)(3 * sizeof(float)));


        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glEnd();
        glDeleteVertexArrays(1, &VAO);

        // Setup for the next frame.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // render your GUI
        ImGui::Begin("Mandelbrot");
        ImGui::Text("Resolution");
        ImGui::RadioButton("512", &resolution, 4); ImGui::SameLine();
        ImGui::RadioButton("1024", &resolution, 2); ImGui::SameLine();
        ImGui::RadioButton("2048", &resolution, 1);

        ImGui::Text("Max Iterations");
        ImGui::InputInt("", &max_iterations);

        struct FuncHolder { 
            static bool ItemGetter(void* data, int idx, const char** out_str) {
                std::vector<std::string>* names = (std::vector<std::string>*)data;
                *out_str = (*names)[idx].c_str();
                return true;
            } 
        };
        ImGui::Combo("Backends", &selected_backend, &FuncHolder::ItemGetter, &compute_backends, (int)compute_backends.size());

        ImGui::ColorEdit3("Color", color);

        std::string frame = "Frame ";
        frame.append(std::to_string(last_frame_time.count() / 1000.0));
        frame.append(" ms.");
        ImGui::Text(frame.c_str());
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    void registerCompute(std::string name) {
        compute_backends.push_back(name);
        selected_backend = 0;
    }

    std::string selectedCompute(void) {
        return compute_backends[selected_backend];
    }
};
Window w;

void scroll_callback(GLFWwindow* window, double xo, double yo) {
    if (!ImGui::GetIO().WantCaptureMouse) {
        w.scroll(xo, yo);
    }
}

int main() {
    Mandelbrot mandelbrot(w.tex_output);

    std::map<std::string, computeBase*> compute;

    compute["CPU float"] = new computeCPU<float>(mandelbrot);
    w.registerCompute("CPU float");
    compute["CPU double"] = new computeCPU<double>(mandelbrot);
    w.registerCompute("CPU double");

    compute["GPU float"] = new computeGPU(mandelbrot);
    w.registerCompute("GPU float");

    while (!glfwWindowShouldClose(w.window))
    {
        glfwPollEvents();
        w.handleInput(mandelbrot);

        if (mandelbrot.resolution != w.resolution) {
            mandelbrot.resolution = w.resolution;
            mandelbrot.dirty = true;
        }

        if (mandelbrot.max_iterations != w.max_iterations) {
            mandelbrot.max_iterations = w.max_iterations;
            mandelbrot.dirty = true;
        }

        if (mandelbrot.backend != w.selectedCompute()) {
            mandelbrot.backend = w.selectedCompute();
            mandelbrot.dirty = true;
        }

        if (memcmp(w.color, w.old_color, sizeof(float) * 3)) {
            uint8_t base[4] = {(uint8_t)(w.color[0] * 255), (uint8_t)(w.color[1] * 255), (uint8_t)(w.color[2] * 255), 0};
            memcpy(w.old_color, w.color, sizeof(float) * 3);
            make_gradient(base, mandelbrot.gradient);
            mandelbrot.dirty = true;
        }

        if (mandelbrot.dirty) {
            std::cout << "Compute frame" << std::endl;
            auto c = compute[std::string(w.selectedCompute())];
            c->compute();
            w.last_frame_time = c->frame_time();
            mandelbrot.dirty = false;
        }
        w.draw();
    }
    exit(EXIT_SUCCESS);

    return 0;
}

