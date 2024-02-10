/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

/*
 * This header absorbs some differences in the OpenGL implementations
 * between the platforms we support. This header is included from the
 * glrender.c file only.
 *
 * GLFW and GLEW work the same way, but we don't use them to reduce
 * our dependencies.
 */

#ifndef SUIKA_GLHELPER_H
#define SUIKA_GLHELPER_H

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

/*
 * For Windows.
 */
#define WGL_CONTEXT_MAJOR_VERSION_ARB		0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB		0x2092
#define WGL_CONTEXT_FLAGS_ARB			0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB		0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB	0x00000001

/*
 * Define the missing macros for OpenGL 2+ and OpenGL ES 2+.
 */
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE			0x812F
#define GL_TEXTURE0				0x84C0
#define GL_TEXTURE1				0x84C1
#define GL_ARRAY_BUFFER				0x8892
#define GL_ELEMENT_ARRAY_BUFFER			0x8893
#define GL_STATIC_DRAW				0x88E4
#define GL_FRAGMENT_SHADER			0x8B30
#define GL_LINK_STATUS				0x8B82
#define GL_VERTEX_SHADER			0x8B31
#define GL_COMPILE_STATUS			0x8B81
#endif

/*
 * Define the missing typedefs if glext.h is not included.
 */
#ifndef __gl_glext_h_
typedef char GLchar;
typedef ssize_t GLsizeiptr;
#endif

/*
 * Declare the OpenGL 2+ API functions as pointer-to-function because:
 *  - Linux: libOpenGL.so provides pure stubs and thus we override them in x11main.c
 *  - Windows: opengl32.dll doesn't export OpenGL 2+ symbols and thus we define them in winmain.c
 *
 * We have to get real API pointers by an extension mechanism:
 *  - Linux: glXGetProcAddress()
 *  - Windows: wglGetProcAddress()
 *
 * With Qt, we use replacement macros and don't define the API symbols directly.
 */
#if (defined(SUIKA_TARGET_POSIX) && !defined(USE_QT)) || defined(SUIKA_TARGET_WIN32)
extern GLuint (APIENTRY *glCreateShader)(GLenum type);
extern void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
extern void (APIENTRY *glCompileShader)(GLuint shader);
extern void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
extern void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
extern void (APIENTRY *glLinkProgram)(GLuint program);
extern void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
extern void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern GLuint (APIENTRY *glCreateProgram)(void);
extern void (APIENTRY *glUseProgram)(GLuint program);
extern void (APIENTRY *glGenVertexArrays)(GLsizei n, GLuint *arrays);
extern void (APIENTRY *glBindVertexArray)(GLuint array);
extern void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
extern void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
extern GLint (APIENTRY *glGetAttribLocation)(GLuint program, const GLchar *name);
extern void (APIENTRY *glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
extern GLint (APIENTRY *glGetUniformLocation)(GLuint program, const GLchar *name);
extern void (APIENTRY *glUniform1i)(GLint location, GLint v0);
extern void (APIENTRY *glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void (APIENTRY *glDeleteShader)(GLuint shader);
extern void (APIENTRY *glDeleteProgram)(GLuint program);
extern void (APIENTRY *glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
extern void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);
#ifdef SUIKA_TARGET_WIN32
/* Note: only Windows lacks glActiveTexture(), libOpenGL.so exports one that actually works. */
extern void (APIENTRY *glActiveTexture)(GLenum texture);
#endif
#endif /* if defined(SUIKA_TARGET_WIN32) || (defined(LINUX) && !defined(USE_QT) */

/*
 * With Qt, we use a wrapper to call the Qt's OpenGL functions.
 *  - We replace all gl*() calls in glrender.c except initialization
 *  - gl*() are replaced by q_gl*() by the preprocessor macros defined below
 *  - q_gl*() are defined in openglwidget.cpp
 *  - We use setup_*() functions for initialization
 */
#ifdef USE_QT
/* OpenGL 1.1 */
#define glViewport q_glViewport
#define glClear q_glClear
#define glClearColor q_glClearColor
#define glFlush q_glFlush
#define glGenTextures q_glGenTextures
#define glPixelStorei q_glPixelStorei
#define glBindTexture q_glBindTexture
#define glTexParameteri q_glTexParameteri
#define glTexParameteri q_glTexParameteri
#define glTexImage2D q_glTexImage2D
#define glActiveTexture q_glActiveTexture
#define glDeleteTextures q_glDeleteTextures
#define glEnable q_glEnable
#define glBlendFunc q_glBlendFunc
#define glDrawElements q_glDrawElements
void q_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void q_glClear(GLbitfield mask);
void q_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void q_glEnable(GLenum cap);
void q_glBlendFunc(GLenum sfactor, GLenum dfactor);
void q_glFlush(void);
void q_glGenTextures(GLsizei n, GLuint *textures);
void q_glActiveTexture(GLenum texture);
void q_glBindTexture(GLenum target, GLuint texture);
void q_glDeleteTextures(GLsizei n, const GLuint *textures);
void q_glPixelStorei(GLenum pname, GLint param);
void q_glTexParameteri(GLenum target, GLenum pname, GLint param);
void q_glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void q_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
/* OpenGL 2+ */
#define glUseProgram q_glUseProgram
#define glBindBuffer q_glBindBuffer
#define glBufferData q_glBufferData
void q_glUseProgram(GLuint program);
void q_glBindBuffer(GLenum target, GLuint buffer);
void q_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
/* OpenGL 3+ */
#define glBindVertexArray q_glBindVertexArray
void q_glBindVertexArray(GLuint array);
#endif	/* ifdef USE_QT */

#endif	/* GLHELPER_H */
