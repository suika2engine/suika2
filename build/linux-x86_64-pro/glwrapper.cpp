/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * OpenGL API wrapper for Qt:
 *  - OpenGL 1.1 API is wrapped by a Qt class and we have to use it
 *  - OpenGL 2+/3+ API is wrapped by some Qt classes and we have to use them
 *
 * [Changes]
 *  2023-09-07 Created.
 */

extern "C" {
#include "suika.h"
};

#include <QOpenGLExtraFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <assert.h>

// Maximum number of fragment shaders we support.
#define FSHADER_COUNT 10

extern "C" {

//
// The sole vertex shader.
//
static QOpenGLShader *vshader;

//
// Number of fragment shaders registered.
//
static int fshader_count;

//
// Object tables.
//
static QOpenGLShader *fshader_tbl[FSHADER_COUNT];
static QOpenGLShaderProgram *prog_tbl[FSHADER_COUNT];
static QOpenGLVertexArrayObject *vao_tbl[FSHADER_COUNT];
static QOpenGLBuffer *vbo_tbl[FSHADER_COUNT];
static QOpenGLBuffer *ibo_tbl[FSHADER_COUNT];

//
// Current selected program.
//
static GLuint cur_program = (GLuint)-1;

/*
//
// Viewport. (referenced by OpenGLWidget::paintGL())
//
namespace glwrapper {
int viewportX;
int viewportY;
int viewportWidth;
int viewportHeight;
};
*/

//
// OpenGL 2+/3+ initialization
//  - Here we do initialization of shader/program/VAO/VBO/IBO
//  - We can't call raw OpenGL 2+ API on Qt in glrender.c
//  - Alternatively, setup_*() is called from init_opengl() in glrender.c
//

//
// Setup a vertex shader.
//
bool setup_vertex_shader(const char **vshader_src, GLuint *ret_vshader)
{
    // Assert the sole vertex shader is not created.
    assert(vshader == NULL);

    // Create the vertex shader.
    vshader = new QOpenGLShader(QOpenGLShader::Vertex);
    vshader->compileSourceCode(*vshader_src);
    if (!vshader->isCompiled()) {
        log_info("Failed to compile a vertex shader: %s", vshader->log().toUtf8().data());
        return false;
    }
    *ret_vshader = vshader->shaderId();

    return true;
}

//
// Cleanup a vertex shader.
//
void cleanup_vertex_shader(GLuint vshader_id)
{
    // Delete the sole vertex shader.
    if (vshader != NULL) {
        assert(vshader->shaderId() == vshader_id);
        delete vshader;
        vshader = NULL;
    }
}

//
// Setup a fragment shader, a program, a VAO, a VBO and an IBO.
//
bool
setup_fragment_shader(
    const char **fshader_src,
    GLuint vshader_id,
    bool use_second_texture,
    GLuint *ret_fshader,        // OUT
    GLuint *ret_prog,           // OUT
    GLuint *ret_vao,            // OUT
    GLuint *ret_vbo,            // OUT
    GLuint *ret_ibo)            // OUT
{
    // Assert non-NULL.
    assert(fshader_src != NULL);
    assert(*fshader_src != NULL);
    assert(ret_fshader != NULL);
    assert(ret_prog != NULL);
    assert(ret_vao != NULL);
    assert(ret_vbo != NULL);
    assert(ret_ibo != NULL);

    // Assert the sole vertex shader is created.
    assert(vshader != NULL);

    // Assert we have room for the tables.
    assert(fshader_count < FSHADER_COUNT);

    // Create a fragment shader.
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment);
    fshader->compileSourceCode(*fshader_src);
    if (!fshader->isCompiled()) {
        log_info("Failed to compile a fragment shader: %s", fshader->log().toUtf8().data());
        return false;
    }
    *ret_fshader = fshader->shaderId();

    // Create a program.
    QOpenGLShaderProgram *prog = new QOpenGLShaderProgram();
    prog->addShader(vshader);
    prog->addShader(fshader);
    prog->link();
    prog->bind();
    *ret_prog = prog->programId();

    // Create a VAO.
    QOpenGLVertexArrayObject *vao = new QOpenGLVertexArrayObject();
    vao->create();
    vao->bind();
    *ret_vao = vao->objectId();

    // Create a VBO for "XYZUVA" * 4vertices.
    QOpenGLBuffer *vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->bind();
    vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo->allocate(6 * 4 * sizeof(GLfloat)); // 6 * 4 = "XYZUVA" * 4vertices
    *ret_vbo = vbo->bufferId();

    // Set the vertex attibute for "a_position" in the vertex shader.
    int posLoc = prog->attributeLocation("a_position");
    prog->enableAttributeArray(posLoc);
    prog->setAttributeBuffer(posLoc,
                             GL_FLOAT,
                             0,                         // pos 0: at "X" in "XYZUVA"
                             3,                         // for "XYZ"
                             6 * sizeof(GLfloat));      // for "XYZUVA"

    // Set the vertex attibute for "a_texCoord" in the vertex shader.
    int texLoc = prog->attributeLocation("a_texCoord");
    prog->enableAttributeArray(texLoc);
    prog->setAttributeBuffer(texLoc,
                             GL_FLOAT,
                             3 * sizeof(GLfloat),       // pos 3: at "U" in "XYZUVA"
                             2,                         // for "UV"
                             6 * sizeof(GLfloat));      // for "XYZUVA"

    // Set the vertex attibute for "a_alpha" in the vertex shader.
    int alphaLoc = prog->attributeLocation("a_alpha");
    prog->enableAttributeArray(alphaLoc);
    prog->setAttributeBuffer(alphaLoc,
                             GL_FLOAT,
                             5 * sizeof(GLfloat),       // pos 5: at "A" in "XYZUVA"
                             1,                         // for "A"
                             6 * sizeof(GLfloat));      // for "XYZUVA"

    // Setup "s_texture" in a fragment shader.
    int samplerLoc = prog->uniformLocation("s_texture");
    prog->setUniformValue(samplerLoc, 0);

    // Setup "s_rule" in a fragment shader if we use a second texture.
    if (use_second_texture) {
        int ruleLoc = prog->uniformLocation("s_rule");
        prog->setUniformValue(ruleLoc, 1);
    }

    // Create an IBO.
    const GLushort indices[] = {0, 1, 2, 3};
    QOpenGLBuffer *ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    ibo->create();
    ibo->bind();
    ibo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    ibo->allocate(4 * sizeof(GLushort));
    ibo->write(0, indices, 4 * sizeof(GLushort));
    *ret_ibo = ibo->bufferId();

    // Store objects to the tables.
    fshader_tbl[fshader_count] = fshader;
    prog_tbl[fshader_count] = prog;
    vao_tbl[fshader_count] = vao;
    vbo_tbl[fshader_count] = vbo;
    ibo_tbl[fshader_count] = ibo;
    fshader_count++;

    return true;
}

//
// Cleanup a fragment shader with a program, a VAO, a VBO and an IBO.
//
void
cleanup_fragment_shader(
    GLuint fshader,
    GLuint prog,
    GLuint vao,
    GLuint vbo,
    GLuint ibo)
{
    // Search the fragment shader index.
    int index;
    for (index = 0; index < fshader_count; index++) {
        assert(fshader_tbl[index] != NULL);
        if (fshader_tbl[index]->shaderId() == fshader)
            break;
    }
    if (index == fshader_count)
        return;

    // Check integity.
    assert(prog_tbl[index]->programId() == prog);
    assert(vao_tbl[index]->objectId() == vao);
    assert(vbo_tbl[index]->bufferId() == vbo);
    assert(ibo_tbl[index]->bufferId() == ibo);

    // Delete all.
    delete prog_tbl[index];
    delete ibo_tbl[index];
    delete vbo_tbl[index];
    delete vao_tbl[index];
    delete fshader_tbl[index];

    // NULLify the table entries.
    fshader_tbl[index] = NULL;
    prog_tbl[index] = NULL;
    vao_tbl[index] = NULL;
    vbo_tbl[index] = NULL;
    ibo_tbl[index] = NULL;
}

}; // extern "C"

//
// OpenGL API 1.1/2+/3+ wrapper
//  - We wrap OpenGL API
//    - Here we define functions named q_gl*()
//    - API calls in glrender.c are repladed to q_gl*() by macros defined in glhelper.h
//  - API 1.1:
//    - Most q_gl*() are simple translations to QOpenGLFunctions::gl*()
//    - Only q_glViewport() has a lazy applying mechanism
//  - API 2+:
//    - We need some Qt's classes for shaders/program/VAO/VBO/IBO
//    - We implement only glUseProgram(), glBindBuffer() and glBufferData()
//  - API 3+
//    - It's a stub because we call glBindVertexArray() in q_glUseProgram()
//
extern "C" {

// F: the abbreviated pointer to the QOpenGLFunctions object.
#define F QOpenGLContext::currentContext()->functions()

// -- OpenGL 1.1 --

void q_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
/*
    // Settings will be applied in a next frame because we can't set a
    // viewport outside paintGL().
    glwrapper::viewportX = (int)x;
    glwrapper::viewportY = (int)y;
    glwrapper::viewportWidth = (int)width;
    glwrapper::viewportHeight = (int)height;
*/
}

void q_glClear(GLbitfield mask)
{
    F->glClear(mask);
}

void q_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    F->glClearColor(red, green, blue, alpha);
}

void q_glEnable(GLenum cap)
{
    F->glEnable(cap);
}

void q_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    F->glBlendFunc(sfactor, dfactor);
}

void q_glFlush(void)
{
    F->glFlush();
}

void q_glGenTextures(GLsizei n, GLuint *textures)
{
    F->glGenTextures(n, textures);
}

void q_glActiveTexture(GLenum texture)
{
    F->glActiveTexture(texture);
}

void q_glBindTexture(GLenum target, GLuint texture)
{
    F->glBindTexture(target, texture);
}

void q_glDeleteTextures(GLsizei n, const GLuint *textures)
{
    F->glDeleteTextures(n,textures);
}

void q_glPixelStorei(GLenum pname, GLint param)
{
    F->glPixelStorei(pname, param);
}

void q_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    F->glTexParameteri(target, pname, param);
}

void q_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    F->glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
}

void q_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    F->glDrawElements(mode, count, type, indices);
}

// -- OpenGL 2+ --

void q_glUseProgram(GLuint program)
{
    // Search the program index that was created in setup_fragment_shader().
    int index;
    for (index = 0; index < fshader_count; index++) {
        assert(prog_tbl[index] != NULL);
        if (prog_tbl[index]->programId() == program)
            break;
    }
    assert(index != fshader_count);
    if (index == fshader_count)
        return;

    // Bind a program.
    prog_tbl[index]->bind();

    // Bind related VAO/VBO/IBO here. Thereby, subsequent
    // calls of glBindVertexArray() and glBindBuffer() are pure stubs
    vao_tbl[index]->bind();
    vbo_tbl[index]->bind();
    ibo_tbl[index]->bind();

    // Select the index for subsequent glBufferData().
    cur_program = index;
}

void q_glBindBuffer(GLenum target, GLuint buffer)
{
    // Already binded by q_glUseProgram().
}

void q_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    // Assert this function is called from draw_elements() only.
    // We write 4 vertices of a triangle strip to VBO.
    assert(target == GL_ARRAY_BUFFER);
    assert(size == 6 * 4 * sizeof(GLfloat));
    assert(usage == GL_STATIC_DRAW);

    // Assert q_glUseProgram() is already called.
    assert(cur_program != (GLuint)-1);

    // Note: "usage" is already set by setUsagePattern().
    // Note: memory space is already allocated by allocate().

    // Write the vertex data.
    vbo_tbl[cur_program]->write(0, data, size);
}

// -- OpenGL 3+ --

void q_glBindVertexArray(GLuint array)
{
    // Already binded by q_glUseProgram().
}

#undef F
}; // extern "C"
