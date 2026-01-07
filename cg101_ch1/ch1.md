# CH1 학습 범위 및 OT 보고서: OpenGL 개요, Rendering Pipeline, 색 변화 Triangle 코드 역할 분석

## 1. CH1에서 학습하는 내용의 범위 정의

CH1의 학습 목표는 “Modern OpenGL(core profile) 기반 렌더링의 최소 실행 경로(minimal viable path)”를 구축하고, CPU–GPU 경계에서 발생하는 데이터/상태/프로그램 흐름을 개념적으로 정렬하는 데 있다. 본 챕터는 3D 수학(선형대수, 행렬 변환, 좌표계 체인)을 도입하지 않으며, 다음 항목만을 정확히 다룬다.

---

1. **OpenGL의 정체**

* OpenGL은 그래픽 하드웨어에 직접 접근하는 “언어”가 아니라, **graphics API specification**에 해당한다.
* 사용자가 호출하는 함수 집합은 **state machine** 관점에서 동작하며, 실제 실행은 **driver implementation**이 수행한다.

2. **Rendering Pipeline의 개요**

* vertex data가 GPU에 적재되고, shader program(= programmable pipeline stages)이 컴파일/링크된 뒤, draw call에 의해 pipeline이 실행되는 전체 흐름을 개념적으로 확립한다.

3. **Modern OpenGL의 필수 오브젝트 모델**

* Buffer Object(VBO), Vertex Array Object(VAO), Shader Object, Program Object, Uniform의 역할과 상호작용을 “삼각형 하나” 수준에서 검증한다.

4. **WSL2 + Ubuntu 환경에서의 실행 가능성 검증**

* `glxinfo -B`를 통해 OpenGL renderer가 GPU 가속 경로인지(llvmpipe 여부 포함) 확인하고, 창 생성/컨텍스트 생성/렌더 루프가 정상 작동함을 확인한다.

5. **시간 기반 색 변화 Triangle의 구현**

* Fragment Shader 색을 uniform으로 공급하고, 매 프레임 시간에 따라 값을 갱신함으로써 “CPU→GPU parameter update”의 최소 예제를 구성한다.

---

## 2. OpenGL OT: OpenGL이 “정확히 무엇인지”에 대한 정의

OpenGL은 Khronos Group이 관리하는 **graphics API specification**이며, 애플리케이션이 2D/3D 그래픽을 생성하기 위해 호출하는 함수들의 의미(semantics)를 정의한다. OpenGL 자체는 구현이 아니라 표준(specification)이며, 실제로는 GPU vendor 또는 플랫폼이 제공하는 **OpenGL driver**가 해당 specification에 맞추어 동작을 구현한다.

OpenGL의 운영 방식은 크게 다음 특성을 갖는다.

* **State machine model**: OpenGL 호출은 “현재 상태(current state)”를 변경하거나, “현재 상태를 전제로” 특정 작업을 수행한다. 예를 들어 `glUseProgram`은 “현재 사용 중인 program object” 상태를 설정하고, `glBindBuffer`는 특정 target(GL_ARRAY_BUFFER 등)에 대해 “현재 바인딩된 buffer object” 상태를 설정한다.

* **Object-based API**: OpenGL은 내부 리소스를 handle(정수 ID)로 표현하며, Buffer Object, Vertex Array Object, Shader Object, Program Object 등의 오브젝트를 생성/바인딩/삭제하는 방식으로 관리한다.

* **Context 기반 실행**: OpenGL 호출은 특정 **OpenGL Context**에 종속된다. Context는 OpenGL 상태와 오브젝트 네임스페이스를 포함하며, window system(예: X11/Wayland/WSLg)과 결합되어 default framebuffer를 제공한다. GLFW는 이 Context 생성 및 window/event 처리의 편의 계층을 제공한다.

* **Core profile mindset**: Modern OpenGL(core profile)에서는 legacy fixed-function pipeline을 전제로 하지 않으며, 렌더링은 shader 기반으로 진행된다. 또한 core profile에서는 VAO 없이 vertex attribute를 사용한 draw가 허용되지 않는 것이 일반적이다.

요약하면 OpenGL은 “GPU에게 그리기 작업을 수행하도록 지시하기 위한 표준화된 명령 체계”이며, CH1은 이 명령 체계가 “데이터 업로드 + shader program + draw call”의 결합으로 어떻게 최소 기능을 달성하는지에 초점을 둔다.

---

## 3. OpenGL Rendering Pipeline: 화면에 무언가를 그리는 처리 흐름

OpenGL 기반 렌더링은 “애플리케이션이 pipeline을 직접 실행하는 것”이 아니라, draw call을 통해 driver에게 “현재 상태와 리소스를 기반으로 pipeline을 실행하라”는 작업을 제출하는 형태로 진행된다. CH1 기준의 핵심 pipeline 흐름은 아래와 같이 정리된다.

### 3.1 Application/Driver 단계 (CPU 영역)

1. **Resource preparation**

* 정점 좌표 배열을 CPU 메모리에 구성한다.
* Buffer Object(VBO)를 생성하고, `glBufferData`로 GPU가 접근 가능한 메모리 영역으로 데이터를 업로드한다.

2. **Vertex input specification**

* VAO를 생성/바인딩하고, `glVertexAttribPointer` 및 `glEnableVertexAttribArray`로 “vertex attribute layout”을 정의한다.
* 이 단계는 GPU가 VBO의 raw bytes를 “어떤 형식의 vertex attribute로 해석할지”를 결정하는 메타데이터를 구축하는 단계이다.

3. **Shader program preparation**

* Vertex Shader와 Fragment Shader를 compile하고, Program Object로 link한다.
* 이후 렌더링 시점에 `glUseProgram`으로 활성화한다.

4. **Draw submission**

* `glDrawArrays` 또는 `glDrawElements` 호출로 draw call을 제출한다.
* 이 시점에 driver는 현재 Context 상태(바인딩된 VAO/VBO, 사용 중인 program, uniform 값, viewport, framebuffer 등)를 사용하여 GPU pipeline을 실행한다.

### 3.2 GPU Pipeline 단계 (Programmable + Fixed stages 혼합)

CH1에서는 개념적으로 다음 흐름만을 확정한다.

1. **Vertex Fetch & Vertex Shader execution**

* VAO에 정의된 layout을 기반으로, VBO에서 attribute를 읽는다(vertex fetch).
* Vertex Shader는 정점마다 실행되며, 핵심 출력은 `gl_Position`(clip-space position)이다. CH1에서는 2D 입력을 그대로 `gl_Position`으로 승격하여 출력한다.

2. **Primitive Assembly, Rasterization**

* 입력 정점들이 triangle primitive로 조립된다.
* Rasterization은 triangle이 화면의 pixel grid에 덮는 영역을 계산하여 fragment 후보를 생성한다.

3. **Fragment Shader execution**

* 생성된 fragment마다 Fragment Shader가 실행되며, 최종 색을 산출한다.
* CH1에서는 uniform `uColor`를 사용하여 시간에 따른 색 변화를 구현한다.

4. **Per-fragment operations & Framebuffer write**

* depth/stencil/blend 등의 후처리 단계가 적용될 수 있으나, CH1에서는 기본값(주로 비활성 상태)에 기반하여 default framebuffer에 색이 기록된다.

5. **Swap buffers**

* double buffering 환경에서 `glfwSwapBuffers`가 front/back buffer 교체를 수행하여 화면에 결과가 나타난다.

---

## 4. 색 변화 Triangle 예제 코드: 코드 블럭 단위 역할 분석

아래 분석은 “색 변화 삼각형” 코드의 기능 단위를 논리 블럭 단위로 분해하여 각 블럭의 역할, OpenGL 상태 변화, 그리고 pipeline 관점에서의 의미를 서술한다. (코드는 CH1 표준 예제로 간주한다.)

### 4.1 Header include 블럭: API 표면과 로더/윈도우 계층 결합

```cpp
#include <cstdio>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
```

* `GLFW/glfw3.h`는 window 생성, OpenGL Context 생성, 입력 처리, 이벤트 루프를 제공한다.
* `glad/glad.h`는 OpenGL 함수 포인터 로딩을 위한 선언을 제공한다. Modern OpenGL 환경에서 OpenGL 함수는 런타임에 로드되는 경우가 일반적이며, GLAD는 `glfwGetProcAddress`를 사용해 해당 함수 포인터를 초기화한다.
* `cstdio`, `cmath`는 로깅 및 `sin` 기반 시간 함수를 사용하기 위한 표준 라이브러리 의존성이다.

### 4.2 Viewport 콜백 블럭: window size 변경에 대한 렌더 타겟 정의 갱신

```cpp
static void framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    (void)window;
    glViewport(0, 0, w, h);
}
```

* `glViewport`는 NDC-to-window 변환에서 사용될 viewport를 설정한다. 결과적으로 rasterization 결과가 기록될 window-space 영역을 규정한다.
* window 리사이즈 시 viewport를 갱신하지 않으면, “렌더 결과가 일부만 보이거나 스케일이 비정상”인 현상이 발생할 수 있다.
* CH1에서는 viewport의 수학적 의미를 확장하지 않으며, “window framebuffer와 화면 매핑의 필수 상태”로만 확정한다.

### 4.3 Shader compile 블럭: Shader Object 생성 및 컴파일 상태 검증

```cpp
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
```

* `glCreateShader`는 Shader Object를 생성한다(Vertex Shader 또는 Fragment Shader).
* `glShaderSource`는 GLSL 소스 코드를 Shader Object에 연결한다.
* `glCompileShader`는 GLSL을 driver 내부 표현으로 컴파일한다. 이 과정은 구현에 따라 JIT compilation 또는 IR 변환 등 다양한 내부 단계를 포함할 수 있다.
* `glGetShaderiv(GL_COMPILE_STATUS)` 및 `glGetShaderInfoLog`는 실패 원인을 확보하기 위한 필수 진단 경로이다. CH1에서 black screen 원인의 상당수가 shader compile 실패이므로, 로깅은 “필수 안전장치”로 간주한다.

### 4.4 Program link 블럭: Program Object 생성, 링크, 셰이더 수명 관리

```cpp
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
```

* `glCreateProgram`은 Program Object를 생성한다. Program Object는 pipeline에서 실행 가능한 shader stage 조합의 결과물이다.
* `glAttachShader`는 Program Object에 Shader Object를 부착한다.
* `glLinkProgram`은 stage 간 인터페이스(예: varying, output/input) 및 최종 실행 가능한 형태를 구성한다.
* 링크 성공 후 `glDeleteShader`를 수행하는 것은 “Shader Object handle의 수명 종료”를 의미하나, Program Object는 링크 결과를 유지한다. 즉, Program Object는 shader source가 아니라 링크된 실행 단위를 보유한다.
* `glGetProgramInfoLog`는 링크 실패 원인(주로 interface mismatch, 버전 문제, 문법 오류)을 확인하는 핵심 진단 경로이다.

### 4.5 GLFW/Context 초기화 블럭: OpenGL 실행 무대의 생성

```cpp
if (!glfwInit()) { ... }

glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

GLFWwindow* window = glfwCreateWindow(800, 600, "CG101 CH1 - Triangle", nullptr, nullptr);
glfwMakeContextCurrent(window);
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { ... }
```

* `glfwInit`는 GLFW subsystem 초기화이다.
* `glfwWindowHint`는 Context 생성 조건을 지정한다. CH1의 기준은 OpenGL 3.3 core profile이며, 이는 programmable pipeline 기반 학습을 위한 최소 버전 전략이다.
* `glfwCreateWindow`는 window 생성과 동시에 해당 window에 결합된 OpenGL Context 생성을 시도한다.
* `glfwMakeContextCurrent`는 이후의 OpenGL 호출이 적용될 Context를 현재 thread에 바인딩한다.
* `gladLoadGLLoader(glfwGetProcAddress)`는 현재 Context에 대해 OpenGL 함수 포인터를 로드한다. 이 단계 이전에 OpenGL 함수를 호출하는 것은 정의되지 않은 동작으로 이어질 수 있다.

### 4.6 GLSL 소스 블럭: pipeline stage 정의(정점→클립 위치, fragment→색)

```cpp
const char* vsSrc = R"GLSL(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)GLSL";
```

* Vertex Shader 입력: `layout(location=0) in vec2 aPos`

  * VAO에서 정의할 vertex attribute 0번 슬롯을 `aPos`로 매핑한다.
* 출력: `gl_Position`

  * clip-space position을 의미하며, pipeline에서 primitive assembly 및 clipping의 기준 좌표가 된다.
  * CH1에서는 2D vec2 입력을 vec4로 승격하여 z=0, w=1로 설정한다.

```cpp
const char* fsSrc = R"GLSL(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 uColor;
    void main() {
        FragColor = vec4(uColor, 1.0);
    }
)GLSL";
```

* Fragment Shader 출력: `out vec4 FragColor`

  * 최종 framebuffer에 기록될 색을 지정한다.
* `uniform vec3 uColor`

  * draw call 실행 시점의 “program의 uniform state”로부터 값을 읽는다.
  * CH1에서는 CPU가 매 프레임 `uColor`를 갱신함으로써 시간 기반 색 변화를 구현한다.

### 4.7 Vertex data 및 VAO/VBO 구성 블럭: GPU 입력 데이터와 해석 규칙의 결합

```cpp
float verts[] = {
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

glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
```

* `verts`는 CPU 메모리 상의 정점 좌표 배열이며, 2D 좌표 3개로 triangle을 정의한다.
* `glGenVertexArrays`, `glGenBuffers`는 VAO/VBO 오브젝트 생성이다.
* `glBindVertexArray(vao)`는 이후의 vertex attribute 관련 상태 설정이 해당 VAO에 기록되도록 한다. core profile에서 VAO는 사실상 “vertex input state의 컨테이너”로 취급된다.
* `glBindBuffer(GL_ARRAY_BUFFER, vbo)` + `glBufferData(...)`는 정점 데이터를 VBO로 업로드한다.
* `glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, offset)`는 다음을 선언한다.

  * attribute location 0은 2개의 float로 구성된 벡터(vec2)이며
  * stride는 정점 1개당 바이트 간격이고(여기서는 2 floats)
  * offset은 버퍼 시작에서의 바이트 오프셋이다.
    이 선언은 “GPU가 VBO의 bytes를 vec2 attribute로 읽는 규칙”을 정의한다.
* `glEnableVertexAttribArray(0)`은 해당 attribute slot을 활성화한다.
* 마지막 unbind는 상태 오염을 줄이기 위한 정리이며, 개념적으로는 “VAO에 필요한 메타데이터가 기록 완료되었다”는 신호로 이해하면 충분하다.

### 4.8 Render loop 블럭: per-frame 상태 갱신, uniform 업데이트, draw, present

```cpp
while (!glfwWindowShouldClose(window)) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    float t = (float)glfwGetTime();
    float g = 0.5f + 0.5f * std::sin(t); // 0..1
    GLint loc = glGetUniformLocation(program, "uColor");
    glUniform3f(loc, 0.2f, g, 0.9f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

* 루프는 “프레임 단위로 반복되는 렌더링 작업”을 수행한다.
* `glClearColor`/`glClear`는 framebuffer를 초기화한다. CH1에서는 depth buffer가 없으므로 `GL_COLOR_BUFFER_BIT`만 초기화한다.
* `glUseProgram(program)`은 현재 pipeline에서 사용할 Program Object를 활성화한다. 이는 OpenGL state machine에서 핵심 상태 전환이며, 이후 draw call은 해당 program을 사용하여 shader stage를 실행한다.
* `glfwGetTime`은 monotonic time을 제공하며, `sin(t)`를 통해 시간 기반 파라미터를 생성한다.
* `glGetUniformLocation`은 program 내 uniform 심볼 `uColor`의 location을 조회한다. location은 driver에 의해 할당되는 인덱스이며, 이후 `glUniform3f` 호출이 해당 uniform 값을 갱신한다.

  * CH1 수준에서는 “uniform 업데이트는 draw call 시 사용되는 program state를 변경한다”로 확정한다.
* `glBindVertexArray(vao)`는 draw call이 참조할 vertex input state를 지정한다.
* `glDrawArrays(GL_TRIANGLES, 0, 3)`는 triangle primitive를 3개의 정점으로 그리도록 제출한다. 이 호출이 실제로 GPU pipeline 실행을 유발한다.
* `glfwSwapBuffers`는 double buffering 환경에서 front/back buffer 교체를 수행하여 결과를 화면에 표시한다.
* `glfwPollEvents`는 window system 이벤트를 처리한다.

### 4.9 Cleanup 블럭: 리소스 수명 종료와 Context 종료

```cpp
glDeleteVertexArrays(1, &vao);
glDeleteBuffers(1, &vbo);
glDeleteProgram(program);

glfwDestroyWindow(window);
glfwTerminate();
```

* OpenGL 오브젝트를 삭제하고 GLFW를 종료한다.
* CH1에서는 메모리 누수/리소스 누적이 치명적이지 않을 수 있으나, “리소스 수명 관리”는 graphics API 사용의 기본 규율로 채택한다.

---

## 5. CH1 결과물의 기술적 의미 요약

CH1의 최종 결과(시간 기반 색 변화 Triangle)는 다음 사실을 실증한다.

* OpenGL Context가 정상적으로 생성되고, GLAD를 통해 함수 포인터 로딩이 완료되었다.
* vertex data가 Buffer Object로 업로드되고, VAO에 의해 “입력 데이터 해석 규칙”이 지정되었다.
* shader program이 compile/link되어 programmable pipeline이 활성화되었다.
* uniform 업데이트가 draw call 실행 결과(Fragment Shader 출력)에 반영됨이 확인되었다.
* rendering loop가 매 프레임 clear → program bind → parameter update → draw → present 순으로 작동한다.

---

## 6. (환경 검증 요약) WSL2에서의 실행 타당성 확인 지표

CH1에서는 다음 지표를 최소 기준으로 사용한다.

* `glxinfo -B` 실행 가능 여부
* `OpenGL core profile version string`이 최소 3.3 이상인지 여부
* `OpenGL renderer string`이 `llvmpipe`인지 여부(학습 지속 가능하나 성능/특성 차이를 인지해야 함)
* `glxgears` 또는 본 예제가 window를 정상 표시하는지 여부