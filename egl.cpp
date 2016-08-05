#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define VERTEX_ARRAY    0
#define UBO_BIND_ITEMS  5

struct ColorF {
  float r, g, b, a;

  ColorF(float r, float g, float b, float a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
};

void TE(int i) {
  GLenum lastError = glGetError();
  if (lastError != GL_NO_ERROR) {
    printf("ERROR %d at %d\n", lastError, i);
    abort();
  }
}

static const EGLint configAttribs[] = {
  EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_DEPTH_SIZE, 8,
  EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
  EGL_NONE
};

static const EGLint contextAttribs[] = {
  EGL_CONTEXT_CLIENT_VERSION, 3,
  EGL_NONE
};

static const int pbufferWidth = 256;
static const int pbufferHeight = 256;

static const EGLint pbufferAttribs[] = {
  EGL_WIDTH, pbufferWidth,
  EGL_HEIGHT, pbufferHeight,
  EGL_NONE,
};

const char *load(const char *filename) {
  FILE *f = fopen(filename, "rb");
  fseek(f, 0, SEEK_END);
  int len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *s = (char *) malloc(len+1);
  fread(s, 1, len, f);
  s[len] = 0;
  fclose(f);
  return s;
}

GLuint initshaders()
{
  GLuint fragmentShader, vertexShader, shaderProgram;

  const char *vsrc[] = {
    "#version 300 es\n",
    "#define WR_VERTEX_SHADER\n",
    load("shared.glsl"),
    load("vs.glsl"),
  };

  const char *fsrc[] = {
    "#version 300 es\n",
    "#define WR_FRAGMENT_SHADER\n",
    load("shared.glsl"),
    load("fs.glsl"),
  };

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, sizeof(fsrc)/sizeof(char *), fsrc, NULL);
  glCompileShader(fragmentShader);
  GLint isShaderCompiled;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isShaderCompiled);
  if (!isShaderCompiled)
  {
    int infoLogLength, charactersWritten;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

    char* infoLog = new char[infoLogLength];
    glGetShaderInfoLog(fragmentShader, infoLogLength, &charactersWritten, infoLog);

    infoLogLength>1 ? printf("%s", infoLog) : printf("Failed to compile fragment shader.");

    delete[] infoLog;
    assert(false);
  }

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, sizeof(vsrc)/sizeof(char *), vsrc, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isShaderCompiled);
  if (!isShaderCompiled)
  {
    int infoLogLength, charactersWritten;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

    char* infoLog = new char[infoLogLength];
    glGetShaderInfoLog(vertexShader, infoLogLength, &charactersWritten, infoLog);

    infoLogLength>1 ? printf("%s", infoLog) : printf("Failed to compile vertex shader.");

    delete[] infoLog;
    assert(false);
  }

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragmentShader);
  glAttachShader(shaderProgram, vertexShader);
  glBindAttribLocation(shaderProgram, VERTEX_ARRAY, "aPosition");
  glLinkProgram(shaderProgram);

  GLint isLinked;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
  if (!isLinked)
  {
    int infoLogLength, charactersWritten;
    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

    char* infoLog = new char[infoLogLength];
    glGetProgramInfoLog(shaderProgram, infoLogLength, &charactersWritten, infoLog);

    infoLogLength>1 ? printf("%s", infoLog) : printf("Failed to link shader program.");

    delete[] infoLog;
    assert(false);
  }

  GLuint index = glGetUniformBlockIndex(shaderProgram, "Items");
  glUniformBlockBinding(shaderProgram, index, UBO_BIND_ITEMS);

  return shaderProgram;
}

int main(int argc, char *argv[])
{
  // 1. Initialize EGL
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;

  eglInitialize(eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg,
                                               pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_ES_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT,
                                       contextAttribs);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  const unsigned char *version = glGetString(GL_VERSION);
  printf("%s\n", version);
  printf("%s\n", glGetString(GL_RENDERER));
  printf("%s\n", glGetString(GL_VENDOR));
  fflush(stdout);

  GLint uv, ubo_size;
  glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &uv);
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &ubo_size);

  printf("ubo size = %d\n", ubo_size);
  printf("vs uniform vectors = %d\n", uv);

  ColorF colors[] = {
    ColorF(1.0, 0.0, 0.0, 1.0),
    ColorF(0.0, 1.0, 0.0, 1.0),
    ColorF(0.0, 0.0, 1.0, 1.0),
    ColorF(1.0, 1.0, 1.0, 1.0),
    ColorF(1.0, 1.0, 0.0, 1.0),
    ColorF(1.0, 0.0, 1.0, 1.0),
    ColorF(0.0, 1.0, 1.0, 1.0),
    ColorF(0.0, 0.0, 0.0, 1.0),
  };

  GLuint ubo;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

  TE(0);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  TE(1);
  glClear(GL_COLOR_BUFFER_BIT);
  TE(2);

  GLfloat vertexData[] = {-0.4f,-0.4f, 0.0f,  // Bottom Left
                           0.4f,-0.4f, 0.0f,  // Bottom Right
                           0.0f, 0.4f, 0.0f}; // Top Middle

  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);
  TE(4);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  TE(5);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
  TE(6);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  TE(7);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  TE(8);
  glBindVertexArray(vao);
  TE(9);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  TE(10);
  glEnableVertexAttribArray(VERTEX_ARRAY);
  TE(11);
  glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, 0);
  TE(12);

  GLuint program = initshaders();

  glUseProgram(program);
  TE(3);

  glBindBufferBase(GL_UNIFORM_BUFFER, UBO_BIND_ITEMS, ubo);

  glDrawArrays(GL_TRIANGLES, 0, 3);
  TE(13);

  static GLubyte pixels[pbufferWidth*pbufferHeight*4];
  glReadPixels(0, 0, pbufferWidth, pbufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  stbi_write_png("test.png", pbufferWidth, pbufferHeight, 4, pixels, pbufferWidth * 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glDeleteBuffers(1, &ubo);
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteVertexArrays(1, &vao);

  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);

  return 0;
}
