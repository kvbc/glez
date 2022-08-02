# GLEZ
Single-header C/C++ GL/GLES wrapper trading performance for convenience

Aiming to make the debugging process much easier 

```cpp
struct glezResource * program = glez_program_create("program", 2,
    glez_shader_createf("vertex shader", "../shaders/vertex.glsl", GL_VERTEX_SHADER),
    glez_shader_createf("fragment shader", "../shaders/fragment.glsl", GL_FRAGMENT_SHADER)
);
```
