// src/main.cpp
#include <cstdio>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


static void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    (void)window;
    glViewport(0, 0, w, h);
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::fprintf(stderr, "Shader compile error:\n%s\n", log);
    }

    return sh;
}

static GLuint makeProgram(const char* vsSrc, const char* fsSrc) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::fprintf(stderr, "Program link error:\n%s\n", log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}


int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "CG101 CH3-1: mat2 transform", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "glfwCreateWindow failed!\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "gladLoadGLLoader failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(1);
    }

    // ---- Shaders: apply mat2 to positions ----
    const char* vsSrc = R"GLSL(
        #version 330 core
        layout (location = 0) in vec2 aPos;

        uniform mat2 uM; // 2x2 linear transform

        void main() {
            vec2 p = uM * aPos;              // linear transform in 2D
            gl_Position = vec4(p, 0.0, 1.0); // lift to clip-space vec4 (no translation here)
        }
    )GLSL";

    const char* fsSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.2, 0.8, 0.9, 1.0);
        }
    )GLSL";

    GLuint program = makeProgram(vsSrc, fsSrc);

    // ---- Triangle vertices (2D) ----
    const float verts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f
    };

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

     // ---- Build a mat2 transform on CPU (static) ----
    // Option A: pure rotation by 30 degrees
    const float deg = 30.0f;
    const float rad = deg * (3.14159265358979323846f / 180.0f);
    const float c = std::cos(rad);
    const float s = std::sin(rad);

    glm::mat2 R(c, -s,
                s,  c);

    // Option B: scale (non-uniform)
    glm::mat2 S(1.2f, 0.0f,
                0.0f, 0.8f);

    // Compose: order matters
    // - M1 = R * S  : scale then rotate (because v' = (R*S)*v = R*(S*v))
    // - M2 = S * R  : rotate then scale
    glm::mat2 M1 = R * S;
    glm::mat2 M2 = S * R;

    // Choose one to observe difference.
    // Start with M1; later switch to M2 and compare.
    glm::mat2 M = M1;

    glUseProgram(program);
    GLint locM = glGetUniformLocation(program, "uM");
    glUniformMatrix2fv(locM, 1, GL_FALSE, glm::value_ptr(M));

    // ---- Render loop ----
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ----
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}