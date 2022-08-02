#ifndef GLEZ_H
#define GLEZ_H

// #include <glad/glad.h>

#ifndef GL_FALSE
#include <GLEZ requires OpenGL headers to be included before the GLEZ header>
#endif

#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

struct glezResource;
enum glezResourceType;

void glez_shader_delete (struct glezResource * res);
void glez_program_delete (struct glezResource * res);
void glez_VAO_delete (struct glezResource * res);
void glez_texture_delete (struct glezResource * res);
static void glez__resource_delete (struct glezResource * res);
static struct glezResource * glez__resource_get (const GLuint id, const enum glezResourceType type, const GLenum target);

/*
 *
 * GLEZ
 * 
 */

#ifdef NDEBUG
#define GLEZ__ASSERT(expr)             ((void)0)
#define GLEZ__ASSERTF(expr, fail_expr) ((void)0)
#define GLEZ__GLCALL(func, ...)        ( func(__VA_ARGS__) )
#define GLEZ__GLCALL_VOID(func, ...)   ( func(__VA_ARGS__), ((void)0) )
#else
#define GLEZ__DEBUG
#define GLEZ__ASSERT(expr)             ( assert((expr)) )
#define GLEZ__ASSERTF(expr, fail_expr) ( (expr) ? ((void)0) : ((fail_expr), GLEZ__ASSERT((expr))) )
#define GLEZ__GLCALL(func, ...)      ((  func(__VA_ARGS__)     ) | (assert(glGetError() == GL_NO_ERROR), 0))
#define GLEZ__GLCALL_VOID(func, ...) (( (func(__VA_ARGS__)), 0 ) | (assert(glGetError() == GL_NO_ERROR), 0))
#endif

#define GLEZ__ASSERT_VAO_BOUND(id)             ( GLEZ__ASSERTF(glezDGL__bound_VAO()           == (GLuint)(id), glezD__log_err_nonbound_resource(GLEZ_RESOURCE_VAO,      GL_NONE,         glezDGL__bound_VAO(),           (GLuint)(id))) )
#define GLEZ__ASSERT_TEXTURE_BOUND(target, id) ( GLEZ__ASSERTF(glezDGL__bound_texture(target) == (GLuint)(id), glezD__log_err_nonbound_resource(GLEZ_RESOURCE_TEXTURE, (GLenum)(target), glezDGL__bound_texture(target), (GLuint)(id))) )

#define GLEZ__ASSERT_RESOURCE(resptr, restype) ( \
        assert( ((const struct glezResource *)(resptr)) != NULL), \
        assert( ((const struct glezResource *)(resptr))->id != 0), \
        assert( ((const struct glezResource *)(resptr))->type == (enum glezResourceType)(restype)) \
    )

#define GLEZ__ENUM(TK) TK ,
#define GLEZ__STR(TK) #TK ,

//
// GL
//

#define glez__GL(func, ...)  GLEZ__GLCALL(func, __VA_ARGS__)
#define glez__GLv(func, ...) GLEZ__GLCALL_VOID(func, __VA_ARGS__)

//
// Resource
//

struct glez__VAO {
    size_t vxsize;
    size_t vxattribs;
    size_t vxattribptr;
    size_t bufstop;
    GLuint * bufs;
};

#define GLEZ__RESOURCE_TYPES(WRAP)  \
    WRAP(GLEZ_RESOURCE_NONE)        \
    WRAP(GLEZ_RESOURCE_SHADER)      \
    WRAP(GLEZ_RESOURCE_PROGRAM)     \
    WRAP(GLEZ_RESOURCE_VAO)         \
    WRAP(GLEZ_RESOURCE_TEXTURE)
enum glezResourceType { GLEZ__RESOURCE_TYPES(GLEZ__ENUM) };
struct glezResource {
    GLuint id;
    GLenum target;
    const char * name;
    enum glezResourceType type;

    struct glez__VAO * vao;

    struct glezResource * next;
};

//
// Error
//

#define GLEZ__ERROR_CODES(WRAP)     \
    WRAP(GLEZ_ERR_NONE)             \
    WRAP(GLEZ_ERR_SHADER_COMPILE)   \
    WRAP(GLEZ_ERR_PROGRAM_LINK)
enum glezErrorCode { GLEZ__ERROR_CODES(GLEZ__ENUM) };
struct glezErrorState {
    jmp_buf jmpbuf;
    enum glezErrorCode code;
    struct glezResource * resource;
    GLchar * errlog;
};

/*
 *
 * Debug
 * 
 */

//
// GL
//

static GLint glezDGL__glGetIntegerv (const GLenum param) {
    GLint val;
    glez__GLv(glGetIntegerv, param, &val);
    return val;
}

static inline GLuint glezDGL__bound_VAO () {
    return glezDGL__glGetIntegerv(GL_VERTEX_ARRAY_BINDING);
}

static GLuint glezDGL__bound_texture (const GLenum target) {
    switch (target) {
        case GL_TEXTURE_2D       : return glezDGL__glGetIntegerv(GL_TEXTURE_BINDING_2D);
        case GL_TEXTURE_CUBE_MAP : return glezDGL__glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP);
    }
    GLEZ__ASSERT(0);
}

//
//
//

static inline const char * glezD__stringify_error_code (const enum glezErrorCode code) {
    static const char * const names[] = { GLEZ__ERROR_CODES(GLEZ__STR) };
    return names[code];
}

static inline const char * glezD__stringify_resource_type (const enum glezResourceType type) {
    static const char * const names[] = { GLEZ__RESOURCE_TYPES(GLEZ__STR) };
    return names[type];
}

static void glezD__log (const char * const frmt, ...) {
#ifdef GLEZ__DEBUG
    puts("============================== DEBUG");
    va_list args;
    va_start(args, frmt);
    vprintf(frmt, args);
    va_end(args);
    puts("");
#endif
}

static void glezD__log_resource (const struct glezResource * const res) {
#ifdef GLEZ__DEBUG
    if (res == NULL)
        puts("resource : NULL");
    else
        printf(
            "resource\n"
            "-- id   : %u\n"
            "-- name : \"%s\"\n"
            "-- type : %s\n",
            res->id,
            res->name,
            glezD__stringify_resource_type(res->type)
        );
#endif
}

static void glezD__log_err_nonbound_resource (const enum glezResourceType type, const GLenum target, const GLuint bound, const GLuint expected) {
    GLEZ__ASSERT(bound != expected);
    puts("Modyfing non-bound resource");
    printf("bound ");
    glezD__log_resource(glez__resource_get(bound, type, target));
    printf("expected ");
    glezD__log_resource(glez__resource_get(expected, type, target));
}

/*
 *
 * State
 * 
 */

struct glez__State {
    struct {
        struct glezResource head;
        struct glezResource * tail;
    } resources;
    void * tofree;
    struct glezErrorState err;
};
static struct glez__State * glez__state () {
    static struct glez__State state = {
        .tofree = NULL,
        .resources.head.next = NULL,
        .resources.tail = &state.resources.head,
        .err = {
            .code = GLEZ_ERR_NONE,
            .resource = NULL,
            .errlog = NULL
        }
    };
    return &state;
}

static inline struct glezErrorState * glez__err () {
    return &(glez__state()->err);
}

static inline void glez__throw (const enum glezErrorCode errcode) {
    GLEZ__ASSERT(errcode != GLEZ_ERR_NONE);

    struct glezErrorState * const err = glez__err();

#ifdef GLEZ__DEBUG
    glezD__log(
        "Encountered an error!\n"
        "code: %s",
        glezD__stringify_error_code(errcode)
    );
    glezD__log_resource(err->resource);
    printf(
        "errlog: %s\n",
        err->errlog
    );
#endif

    // to avoid warnings about resource
    // not being manually deleted
    glez__resource_delete(err->resource);

    err->code = errcode;
    longjmp(err->jmpbuf, 1);
}

//
// Public
//

struct glezErrorState * glez_error_state () {
    return glez__err();
}

void glez_free () {
    struct glez__State * const state = glez__state();

    free(state->tofree);
    free(state->err.errlog);

    struct glezResource * head = &state->resources.head;
    struct glezResource * prev = NULL;
    while (head->next != NULL) {
        // resource not deleted
        if (head->type != GLEZ_RESOURCE_NONE) {
            glezD__log("Resource has not been manually deleted");
            glezD__log_resource(head);
            glez__resource_delete(head);
        }

        head = head->next;
        free(prev);
        prev = head;
    }
}

/*
 *
 * Resource
 * 
 */

static struct glezResource * glez__resource_get (const GLuint id, const enum glezResourceType type, const GLenum target) {
    if (id == 0)
        return NULL;

    struct glezResource * head = &glez__state()->resources.head;
    while (head->next != NULL) {
        if (head->id == id)
        if (head->type == type)
        if (head->target == target)
            return head;
        head = head->next;
    }

    return NULL;
}

static struct glezResource * glez__resource_set (
    const GLuint id,
    const enum glezResourceType type,
    const char * const name,
    const GLenum target
) {
    GLEZ__ASSERT(id != 0);
    GLEZ__ASSERT(type != GLEZ_RESOURCE_NONE);

    struct glezResource * const old_res = glez__resource_get(id, type, target);
    struct glezResource * res = old_res;

    if (res == NULL)
        res = glez__state()->resources.tail;

    res->id = id;
    res->target = target;
    res->type = type;
    res->name = name;

    if (type == GLEZ_RESOURCE_VAO) {
        res->vao = malloc(sizeof(*res->vao)); // freed in glez_VAO_delete()
        GLEZ__ASSERT(res->vao != NULL);
        res->vao->vxsize = 0;
        res->vao->vxattribs = 0;
        res->vao->vxattribptr = 0;
        res->vao->bufstop = 0;
        res->vao->bufs = NULL;
    }

    if (res == old_res)
        return res;

    res->next = malloc(sizeof(struct glezResource)); // freed in glez_free()
    GLEZ__ASSERT(res->next != NULL);
    res->next->next = NULL;

    glez__state()->resources.tail = res->next;

    return res;
}

static void glez__resource_delete (struct glezResource * res) {
    GLEZ__ASSERT(res != NULL);
    GLEZ__ASSERT(res->type != GLEZ_RESOURCE_NONE);

    switch (res->type) {
    case GLEZ_RESOURCE_SHADER:
        glez_shader_delete(res);
        break;
    case GLEZ_RESOURCE_PROGRAM:
        glez_program_delete(res);
        break;
    case GLEZ_RESOURCE_VAO:
        glez_VAO_delete(res);
        break;
    case GLEZ_RESOURCE_TEXTURE:
        glez_texture_delete(res);
        break;
    }
}

/*
 *
 * File
 * 
 */

char * glez__file_read (const char * const name) {
    GLEZ__ASSERT(name != NULL);

    FILE * f = fopen(name, "rb");
    GLEZ__ASSERT(f != NULL);

    GLEZ__ASSERT(fseek(f, 0, SEEK_END) == 0);
    long flen = ftell(f);
    GLEZ__ASSERT(fseek(f, 0, SEEK_SET) == 0);

    char * buf = malloc(sizeof(char) * (flen + 1));
    GLEZ__ASSERT(buf != NULL);
    buf[flen] = '\0';

    GLEZ__ASSERT(fread(buf, sizeof(char), flen, f) == flen);

    GLEZ__ASSERT(fclose(f) == 0);
    return buf;
}

/*
 *
 * Shader
 * 
 */

struct glezResource * glez_shader_create (const char * const name, const GLchar * const src, const GLenum type) {
    GLEZ__ASSERT(src != NULL);

    const GLuint id = glez__GL(glCreateShader, type);
    GLEZ__ASSERT(id != 0);
    struct glezResource * res = glez__resource_set(id, GLEZ_RESOURCE_SHADER, name, GL_NONE);

    glez__GLv(glShaderSource, id, 1, &src, NULL);

    glez__GLv(glCompileShader, id);
    GLint compiled;
    glez__GLv(glGetShaderiv, id, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        GLint infologlen; // null-term included
        glez__GLv(glGetShaderiv, id, GL_INFO_LOG_LENGTH, &infologlen);

        GLchar * infolog = NULL;
        if (infologlen != 0) {
            infolog = malloc(sizeof(GLchar) * infologlen); // freed in glez_free()
            glez__GLv(glGetShaderInfoLog, id, infologlen, NULL, infolog);
        }

        glez__err()->resource = res;
        glez__err()->errlog = infolog;
        glez__throw(GLEZ_ERR_SHADER_COMPILE);
    }

    return res;
}

void glez_shader_delete (struct glezResource * res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_SHADER);
    res->type = GLEZ_RESOURCE_NONE;
    glez__GLv(glDeleteShader, res->id);
}

struct glezResource * glez_shader_createf (const char * const name, const char * const filename, const GLenum type) {
    char * const src = glez__file_read(filename);
    glez__state()->tofree = src;
    struct glezResource * const res = glez_shader_create(name, src, type);
    free(src);
    glez__state()->tofree = NULL;
    return res;
}

/*
 *
 * Program
 * 
 */

struct glezResource * glez_program_create (const char * const name, size_t shader_count, ...) {
    const GLuint id = glez__GL(glCreateProgram);
    GLEZ__ASSERT(id != 0);
    struct glezResource * const res = glez__resource_set(id, GLEZ_RESOURCE_PROGRAM, name, GL_NONE);

    if (shader_count == 0)
        return res;

    va_list shaders;
    va_start(shaders, shader_count);
    for (size_t i = 0; i < shader_count; i++)
        glez__GLv(glAttachShader, res->id, va_arg(shaders, struct glezResource *)->id);
    va_end(shaders);

    glez__GLv(glLinkProgram, id);
    GLint linked;
    glez__GLv(glGetProgramiv, id, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        GLint infologlen; // null-term included
        glez__GLv(glGetProgramiv, id, GL_INFO_LOG_LENGTH, &infologlen);

        GLchar * infolog = NULL;
        if (infologlen != 0) {
            infolog = malloc(sizeof(GLchar) * infologlen); // freed in glez_free()
            glez__GLv(glGetProgramInfoLog, id, infologlen, NULL, infolog);
        }

        glez__err()->resource = res;
        glez__err()->errlog = infolog;
        glez__throw(GLEZ_ERR_PROGRAM_LINK);
    }

    return res;
}

void glez_program_delete (struct glezResource * res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_PROGRAM);
    res->type = GLEZ_RESOURCE_NONE;
    glez__GLv(glDeleteProgram, res->id);
}

void glez_program_use (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_PROGRAM);
    glez__GLv(glUseProgram, res->id);
}

/*
 *
 * VAO
 * 
 */

struct glezResource * glez_VAO_create (const char * const name, size_t vxsize) {
    GLuint id;
    glez__GLv(glGenVertexArrays, 1, &id);
    GLEZ__ASSERT(id != 0);
    struct glezResource * const res = glez__resource_set(id, GLEZ_RESOURCE_VAO, name, GL_NONE);
    res->vao->vxsize = vxsize;
    return res;
}

void glez_VAO_delete (struct glezResource * res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_VAO);

    res->type = GLEZ_RESOURCE_NONE;
    glez__GLv(glDeleteVertexArrays, 1, &(res->id));

    if (res->vao->bufstop > 0)
        glez__GLv(glDeleteBuffers, res->vao->bufstop, res->vao->bufs);

    free(res->vao->bufs);
    free(res->vao);
}

GLuint glez_VAO_add_buffer (
    struct glezResource * const res,
    const GLenum target,
    const GLsizeiptr size,
    const void * const data,
    const GLenum usage
) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_VAO);
    GLEZ__ASSERT_VAO_BOUND(res->id);

    if (res->vao->bufs == NULL) {
        GLEZ__ASSERT(res->vao->bufstop == 0);
        res->vao->bufs = malloc(sizeof(*res->vao->bufs)); // freed in glez_VAO_delete()
    }
    else {
        GLEZ__ASSERT(res->vao->bufstop != 0);
        res->vao->bufs = realloc(res->vao->bufs, res->vao->bufstop * sizeof(*res->vao->bufs));
    }
    GLEZ__ASSERT(res->vao->bufs != NULL);
    GLuint * buf = &(res->vao->bufs[res->vao->bufstop]);
    res->vao->bufstop++;

    glez__GLv(glGenBuffers, 1, buf);
    glez__GLv(glBindBuffer, target, *buf);
    glez__GLv(glBufferData, target, size, data, usage);

    return *buf;
}

void glez_VAO_add_vertex_attrib (
    struct glezResource * const res,
    const GLint size,
    const GLenum type,
    const size_t type_size,
    const GLboolean normalized
) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_VAO);
    GLEZ__ASSERT_VAO_BOUND(res->id);
    assert(res->vao->vxsize != 0);

    glez__GLv(glVertexAttribPointer, res->vao->vxattribs, size, type, normalized, res->vao->vxsize, (const void *)res->vao->vxattribptr);
    glez__GLv(glEnableVertexAttribArray, res->vao->vxattribs);

    res->vao->vxattribs++;
    res->vao->vxattribptr += size * type_size;
}

void glez_VAO_bind (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_VAO);
    glez__GLv(glBindVertexArray, res->id);
}

void glez_VAO_unbind (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_VAO);
    GLEZ__ASSERT_VAO_BOUND(res->id);
#ifdef GLEZ__DEBUG
    glez__GLv(glBindVertexArray, 0);
#endif
}

/*
 *
 * Texture
 * 
 */

void glez_texture_bind (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);
    glez__GLv(glBindTexture, res->target, res->id);
}

void glez_texture_unbind (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);
    GLEZ__ASSERT_TEXTURE_BOUND(res->target, res->id);
#ifdef GLEZ__DEBUG
    glez__GLv(glBindTexture, res->target, 0);
#endif
}

struct glezResource * glez_texture_create (
    const char * const name,
    const GLenum target,
    const GLint min_filter,
    const GLint mag_filter,
    const GLint wrap_s,
    const GLint wrap_t
) {
    GLuint id;
    glez__GLv(glGenTextures, 1, &id);
    GLEZ__ASSERT(id != 0);
    struct glezResource * const res = glez__resource_set(id, GLEZ_RESOURCE_TEXTURE, name, target);

    if (min_filter || mag_filter || wrap_s || wrap_t) {
        glez_texture_bind(res);
        if (min_filter != 0) glez__GLv(glTexParameteri, res->target, GL_TEXTURE_MIN_FILTER, min_filter);
        if (mag_filter != 0) glez__GLv(glTexParameteri, res->target, GL_TEXTURE_MAG_FILTER, mag_filter);
        if (wrap_s     != 0) glez__GLv(glTexParameteri, res->target, GL_TEXTURE_WRAP_S,     wrap_s);
        if (wrap_t     != 0) glez__GLv(glTexParameteri, res->target, GL_TEXTURE_WRAP_T,     wrap_t);
        glez_texture_unbind(res);
    }

    return res;
}

void glez_texture_delete (struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);

    res->type = GLEZ_RESOURCE_NONE;
    glez__GLv(glDeleteTextures, 1, &(res->id));
}

void glez_texture_set_image2D (
    const struct glezResource * const res,
    const GLenum cubemap_target, // for res->target == GL_TEXTURE_CUBE_MAP
    const GLint level,
    const GLint internal_format,
    const GLsizei width,
    const GLsizei height,
    const GLint border,
    const GLenum format,
    const GLenum type,
    const void * const data
) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);
    GLEZ__ASSERT_TEXTURE_BOUND(res->target, res->id);

    GLenum target = res->target;

    if (target == GL_TEXTURE_2D)
        GLEZ__ASSERT(cubemap_target == GL_NONE);
    else if (target == GL_TEXTURE_CUBE_MAP)
        target = cubemap_target;
    else
        GLEZ__ASSERT(0);
    glez__GLv(glTexImage2D, target, level, internal_format, width, height, border, format, type, data);
}

void glez_texture_bind_active (const struct glezResource * const res, const GLenum slot) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);
    glez__GLv(glActiveTexture, slot);
    glez__GLv(glBindTexture, res->target, res->id);
}

void glez_texture_generate_mipmap (const struct glezResource * const res) {
    GLEZ__ASSERT_RESOURCE(res, GLEZ_RESOURCE_TEXTURE);
    GLEZ__ASSERT_TEXTURE_BOUND(res->target, res->id);
    glez__GLv(glGenerateMipmap, res->target);
}

#endif // GLEZ_H
