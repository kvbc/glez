# GLEZ
Single-header C/C++ GL/GLES wrapper trading performance for convenience

Aiming to make the debugging process much easier 

```c
struct glezResource * program = glez_program_create("program", 2, // create a program with debug name "program" and 2 shaders
    glez_shader_createf("vertex shader", "shaders/vertex.glsl", GL_VERTEX_SHADER), // create a vertex shader with debug name "vertex shader" from the specified file
    glez_shader_createf("fragment shader", "shaders/fragment.glsl", GL_FRAGMENT_SHADER) // create a fragment shader with debug name "fragment shader" from the specified file
);
```
