# GLEZ
Single-header C/C++ GL/GLES wrapper trading performance for convenience

Aiming to make the debugging process much easier 

What if your program linking fails? well just delete the shaders I guess
BUT WHAT IF YOU HAVE MULTIPLE PROGRAMS? how do you delete the other programs that came before it and all of its shaders?!
As a perfectionist and no-boilerplate enthusiast, this and many other issues have been bothering me for way too long, so I've decided to make this little handy-dandy library

# Examples

```c
struct glezResource * program = glez_program_create("program", 2,
    glez_shader_createf("vertex shader", "shaders/vertex.glsl", GL_VERTEX_SHADER),
    glez_shader_createf("fragment shader", "shaders/fragment.glsl", GL_FRAGMENT_SHADER)
);
```

Corresponding "raw" OpenGL code

```c
char * read_file (const char * const name) {
    FILE * const f = fopen(name, "rb");
    fseek(f, 0, SEEK_END);
    long flen = ftell(f);
    fseek(f, 0, SEEK_SET);
    char * buf = malloc(sizeof(char) * (flen + 1));
    buf[flen] = '\0';
    fread(buf, sizeof(char), flen, f);
    fclose(f);
    return buf;
}

GLuint create_shader (const GLenum type, const char * const src) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, src, NULL);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint infologlen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologlen);
        if (infologlen != 0) {
            GLchar * const infolog = malloc(sizeof(GLchar) * infologlen);
            glGetShaderInfoLog(shader, infologlen, NULL, infolog);
            printf("shader whoopsie!\n%s\n", infolog);
            free(infolog);
        }
    }
    return shader;
}

GLuint create_program () {
    char * src = read_file("shaders/vertex.glsl");
    const GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, src);
    free(src);
    src = read_file("shaders/fragment.glsl");
    const GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, src);
    free(src);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint infologlen; // null-term included
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologlen);
        if (infologlen != 0) {
            GLchar * const infolog = malloc(sizeof(GLchar) * infologlen); // freed in glez_free()
            glGetProgramInfoLog(program, infologlen, NULL, infolog);
            printf("program whoopsie!\n%s\n", infolog);
            free(infolog);
        }
    }
}
```
