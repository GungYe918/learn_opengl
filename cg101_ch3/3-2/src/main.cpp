// src/main.cpp
#include <cstdio>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


static void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "CG101 CH3-2: mat3 affine (2D)", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "glfwCreateWindow failed!\n");
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

    const char* vsSrc = R"GLSL(
        #version 330 core
        layout (location = 0) in vec2 aPos;

        uniform mat3 uM; // 2D affine transform in homogeneous coordinates

        void main() {
            vec3 p  = vec3(aPos, 1.0); // point: w=1
            vec3 tp = uM * p;          // transformed homogeneous point
            gl_Position = vec4(tp.xy, 0.0, 1.0);
        }
    )GLSL";

    const char* fsSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.95, 0.65, 0.20, 1.0);
        }
    )GLSL";

    GLuint program = makeProgram(vsSrc, fsSrc);

    const float verts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f
    };

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- Build 2D affine matrix M (mat3) ---
    // affine transform의 표준 형태는 F(v)=A v + t 이다
    // A(회전/스케일/쉬어 등 선형 부분) + t(translation 상수항) 이므로 "선형변환"이 아니다
    // 동차좌표를 쓰면 [A t; 0 1] 형태의 3x3으로 만들어 행렬 곱으로 통일할 수 있다
    //
    // compose: M = T * R * S
    // column vector 기준: p' = M p = (T * R * S) p
    // 즉 S 먼저 적용, 그 다음 R, 마지막에 T가 적용된다 (곱셈 순서가 적용 순서의 역)
    float deg = 25.0f;
    float rad = deg * (3.14159265358979323846f / 180.0f);
    float c = std::cos(rad);
    float s = std::sin(rad);

    // 2D scale을 3x3(동차좌표)로 확장
    // 마지막 행/열은 w를 보존하기 위한 장치이며, 점/방향 구분(w=1/0)을 가능하게 한다
    glm::mat3 S(
        1.3f, 0.0f, 0.0f,
        0.0f, 0.9f, 0.0f,
        0.0f, 0.0f, 1.0f
    );

    // 2D rotation을 3x3(동차좌표)로 확장
    // 선형 부분 A는 좌상단 2x2에 들어간다
    glm::mat3 R(
        c,   -s,  0.0f,
        s,    c,  0.0f,
        0.0f, 0.0f, 1.0f
    );

    // 2D translation을 3x3으로 표현
    // 이때 핵심은 "마지막 열"에 (tx, ty)가 들어간다는 것 (uM * vec3(x,y,1)을 전제로 함)
    // w=1인 점에는 tx,ty가 더해지고, w=0인 방향에는 tx,ty가 0배되어 사라진다
    float tx = 0.25f;
    float ty = 0.10f;
    glm::mat3 T(
        1.0f, 0.0f, tx,
        0.0f, 1.0f, ty,
        0.0f, 0.0f, 1.0f
    );

    // 합성: p' = (T * R * S) p
    glm::mat3 M = T * R * S;

    glUseProgram(program);
    GLint locM = glGetUniformLocation(program, "uM");
    glUniformMatrix3fv(locM, 1, GL_FALSE, glm::value_ptr(M));

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}