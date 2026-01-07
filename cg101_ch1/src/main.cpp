// src/main.cpp
#include <cstdio>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// window 크기 변경 시, 렌더링 결과가 기록될 viewport(화면 영역)를 갱신한다.
static void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    (void)window;           // 미사용 경고 제거
    glViewport(0, 0, w, h); // (x, y, width, height)
}

// GLSL 소스(문자열)를 받아 Shader Object를 생성/컴파일하고,
// 컴파일 실패 시 info log를 출력한다.
static GLuint compileShader(GLenum type, const char* src) {
    // Shader Object 생성 (type: GL_VERTEX_SHADER 또는 GL_FRAGMENT_SHADER)
    GLuint sh = glCreateShader(type);
    // GLSL 소스 연결
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    // 컴파일 결과 확인
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, 1024, nullptr, log);
        std::fprintf(stderr, "Shader compile error:\n%s\n", log);
    }

    return sh;
}

// Vertex Shader + Fragment Shader를 컴파일한 뒤 Program Object로 링크
static GLuint makeProgram(const char* vsSrc, const char* fsSrc) {
    // 각 stage의 Shader Object 생성/컴파일
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    // Program Object 생성 및 shader 부착
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    // 링크: stage 간 인터페이스 검증 + 실행 가능한 program 구성
    glLinkProgram(prog);

     // 링크 상태 확인
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
    // -----------------------------
    // 1) GLFW 초기화 + Context 생성 준비
    // -----------------------------
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to init GLFW\n");
        exit(1);
    }

    // Modern OpenGL(core profile) 사용: 3.3 core
    // (fixed-function pipeline 가정 없음, VAO 없이 attribute)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window 생성과 동시에 해당 window에 결합된 OpenGL Context 생성 시도
    GLFWwindow* window = glfwCreateWindow(800, 600, "CG101 CH1 - Triangle", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to Create window!\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // -----------------------------
    // 2) GLAD 로딩: OpenGL 함수 포인터 초기화
    // -----------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to load GLAD!\n");
        exit(1);
    }

    // -----------------------------
    // 3) GLSL: pipeline stage 정의
    // -----------------------------
    // Vertex Shader: VAO의 location=0 attribute(vec2)를 받아 clip-space 위치로 출력
    const char* vsSrc = R"GLSL(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )GLSL";

    // Fragment Shader: uniform uColor를 받아 최종 색을 framebuffer에 기록
    const char* fsSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )GLSL";

    // Program Object 생성 (컴파일+링크)
    GLuint program = makeProgram(vsSrc, fsSrc);

    // -----------------------------
    // 4) 정점 데이터 준비 + VAO/VBO 구성
    // -----------------------------
    // CPU 메모리의 vertex array: 2D 좌표 3개 = 삼각형 1개
    float verts[] = {
        -0.5f, -0.5f,
        0.5f,  -0.5f,
        0.0f,  0.5f
    };

    // VAO: vertex input state 컨테이너
    // VBO: 실제 정점 데이터 저장 (buffer object)
    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // VAO 바인딩: 지금부터 설정하는 vertex attribute 관련 상태를 이 VAO에 기록
    glBindVertexArray(vao);

    // VBO를 GL_ARRAY_BUFFER 타겟으로 바인딩 후 데이터 업로드
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

     // attribute location 0:
    // - 구성: 2개의 float (vec2)
    // - stride: 정점 1개당 바이트 간격 (2 floats)
    // - offset: 버퍼 시작부터의 오프셋(0)
    // => GPU가 VBO의 raw bytes를 vec2 attribute로 "해석"하는 규칙
    glVertexAttribPointer(
        0,                  // location = 0
        2,                  // vec2 = 2 components
        GL_FLOAT,           // component type
        GL_FALSE,           // normalized?
        2 * sizeof(float),  // stride
        (void*)0            // offset
    );

    // 해당 attribute 슬롯 활성화
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // -----------------------------
    // 5) Render loop: clear -> program bind -> uniform update -> draw -> present
    // -----------------------------
    while (!glfwWindowShouldClose(window)) {
        // ESC로 종료
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // 프레임버퍼 초기화 (배경색 설정 후 color buffer clear)
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 현재 pipeline에서 사용할 Program 활성화
        glUseProgram(program);

        // 시간 기반 파라미터 생성: sin(t)를 0..1 범위로 변환
        float t = (float)glfwGetTime();
        float g = 0.5f + 0.5f * std::sin(t); // 0..1
        
        GLint loc = glGetUniformLocation(program, "uColor");
        glUniform3f(loc, 0.2f, g, 0.9f);

        // draw call이 참조할 vertex input state(VAO) 지정
        glBindVertexArray(vao);

        // Draw submission: 이 호출이 실제로 GPU pipeline 실행을 유발
        // - mode: triangles
        // - first: 0번 정점부터
        // - count: 3개 정점
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // double buffering: back buffer에 그린 결과를 front buffer로 교체해 표시
        glfwSwapBuffers(window);

        // window system 이벤트 처리
        glfwPollEvents();
    }

    // -----------------------------
    // 6) Cleanup: 리소스 수명 종료
    // -----------------------------
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
