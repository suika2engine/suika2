/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-06 Created.
 *  2023-09-05 Updated for Qt6.
 */

#include "suika.h"
#include "glrender.h"

/*
 * Include headers.
 */

/*
 * Windows
 *  - We use OpenGL 3.2
 */
#if defined(WIN)
#include <windows.h>
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * macOS
 *  - We use OpenGL 3.2
 */
#if defined(OSX)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif

/*
 * iOS
 *  - We use OpenGL ES 3.0
 */
#if defined(IOS)
#define GL_SILENCE_DEPRECATION
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

/*
 * Android
 *  - We use OpenGL ES 3.0
 */
#if defined(ANDROID)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Emscripten
 *  - We use OpenGL ES 3.0
 */
#if defined(EM)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Linux (excluding Qt)
 *  - We use OpenGL 3.2
 */
#if defined(LINUX) && !defined(USE_QT)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * Qt
 *  - We use a wrapper for QOpenGLFunctions class
 */
#if defined(USE_QT)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * Switch
 *  - We use OpenGL ES 3.0
 */
#if defined(SWITCH)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * SDL2
 *  - We simply use GLEW because the SDL2 port is just for porting base.
 */
#if defined(USE_SDL2_OPENGL)
#include <GL/glew.h>
#endif

/*
 * Program per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint program;

/* For the character dimming. (RGB 50%) */
static GLuint program_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint program_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint program_melt;

/*
 * vertex shader.
 */
static GLuint vertex_shader;

/*
 * Fragment shaders.
 */

/* The normal alpha blending. */
static GLuint fragment_shader;

/* The character dimming. (RGB 50%) */
static GLuint fragment_shader_dim;

/* The rule shader. (1-bit universal transition) */
static GLuint fragment_shader_rule;

/* The melt shader. (8-bit universal transition) */
static GLuint fragment_shader_melt;

/*
 * Vertex array per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vertex_array;

/* For the character dimming. (RGB 50%) */
static GLuint vertex_array_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vertex_array_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vertex_array_melt;

/*
 * Vertex buffer per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vertex_buf;

/* For the character dimming. (RGB 50%) */
static GLuint vertex_buf_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vertex_buf_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vertex_buf_melt;

/*
 * Index buffer per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint index_buf;

/* For the character dimming. (RGB 50%) */
static GLuint index_buf_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint index_buf_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint index_buf_melt;

/*
 * The vertex shader source.
 */
static const char *vertex_shader_src =
#if !defined(EM)
	"#version 100                 \n"
#endif
	"attribute vec4 a_position;   \n"
	"attribute vec2 a_texCoord;   \n"
	"attribute float a_alpha;     \n"
	"varying vec2 v_texCoord;     \n"
	"varying float v_alpha;       \n"
	"void main()                  \n"
	"{                            \n"
	"  gl_Position = a_position;  \n"
	"  v_texCoord = a_texCoord;   \n"
	"  v_alpha = a_alpha;         \n"
	"}                            \n";

/*
 * Fragmen shader sources.
 */

/* The normal alpha blending shader. */
static const char *fragment_shader_src =
#if !defined(EM)
	"#version 100                                        \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  tex.a = tex.a * v_alpha;                          \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The character dimming shader. (RGB 50%) */
static const char *fragment_shader_dim_src =
#if !defined(EM)
	"#version 100                                        \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"uniform sampler2D s_texture;                        \n"
	"void main()                                         \n"
	"{                                                   \n"
	"  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  tex.r = tex.r * 0.5;                              \n"
	"  tex.g = tex.g * 0.5;                              \n"
	"  tex.b = tex.b * 0.5;                              \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The rule shader. (1-bit universal transition) */
static const char *fragment_shader_rule_src =
#if !defined(EM)
	"#version 100                                        \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"uniform sampler2D s_rule;                           \n"
	"void main()                                         \n"
	"{                                                   \n"
        "  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  vec4 rule = texture2D(s_rule, v_texCoord);        \n"
	"  tex.a = 1.0 - step(v_alpha, rule.b);              \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/* The melt shader. (8-bit universal transition) */
static const char *fragment_shader_melt_src =
#if !defined(EM)
	"#version 100                                        \n"
#endif
	"precision mediump float;                            \n"
	"varying vec2 v_texCoord;                            \n"
	"varying float v_alpha;                              \n"
	"uniform sampler2D s_texture;                        \n"
	"uniform sampler2D s_rule;                           \n"
	"void main()                                         \n"
	"{                                                   \n"
        "  vec4 tex = texture2D(s_texture, v_texCoord);      \n"
	"  vec4 rule = texture2D(s_rule, v_texCoord);        \n"
	"  tex.a = clamp((1.0 - rule.b) + (v_alpha * 2.0 - 1.0), 0.0, 1.0); \n"
	"  gl_FragColor = tex;                               \n"
	"}                                                   \n";

/*
 * Internal texture management struct.
 *  - We call glGenTextures() when a texture first unlocked
 */
struct texture {
	/* Texture ID. */
	GLuint id;

	/* This shows whether glGenTextures() was called. */
	bool is_initialized;
};

/*
 * Just for your information, this is our vertex format.
 */
#if 0
struct vertex {
	float x, y, z;
	float u, v;
	float alpha;
};
#endif

/*
 * Forward declaration.
 */
static void draw_elements(int dst_left, int dst_top,
			  struct image * RESTRICT src_image,
			  struct image * RESTRICT rule_image,
			  bool is_dim, bool is_melt,
			  int width, int height,
			  int src_left, int src_top,
			  int alpha, int bt);

/*
 * Initialize the Suika2's OpenGL rendering subsystem.
 */
bool init_opengl(void)
{
	const GLushort indices[] = {0, 1, 2, 3};
	GLint pos_loc, tex_loc, alpha_loc, sampler_loc, rule_loc;

	/* ビューポートを設定する */
	glViewport(0, 0, conf_window_width, conf_window_height);

	/* 頂点シェーダを作成する */
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
	glCompileShader(vertex_shader);

	GLint compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char buf[1024];
		int len;
		log_info("Vertex shader compile error");
		glGetShaderInfoLog(vertex_shader, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/*
	 * TODO: the code below is very redundant and should be refactored.
	 */

	/* フラグメントシェーダ(通常)を作成する */
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char buf[1024];
		int len;
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(fragment_shader, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* フラグメントシェーダ(DIM)を作成する */
	fragment_shader_dim = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_dim, 1, &fragment_shader_dim_src, NULL);
	glCompileShader(fragment_shader_dim);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char buf[1024];
		int len;
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(fragment_shader, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* フラグメントシェーダ(ルール)を作成する */
	fragment_shader_rule = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_rule, 1,
		       &fragment_shader_rule_src, NULL);
	glCompileShader(fragment_shader_rule);

	glGetShaderiv(fragment_shader_rule, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char buf[1024];
		int len;
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(fragment_shader_rule, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* フラグメントシェーダ(メルト)を作成する */
	fragment_shader_melt = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_melt, 1,
		       &fragment_shader_melt_src, NULL);
	glCompileShader(fragment_shader_melt);

	glGetShaderiv(fragment_shader_melt, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		char buf[1024];
		int len;
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(fragment_shader_melt, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* プログラム(通常)を作成する */
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		char buf[1024];
		int len;
		log_info("Program link error\n");
		glGetProgramInfoLog(program, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* プログラム(DIM)を作成する */
	program_dim = glCreateProgram();
	glAttachShader(program_dim, vertex_shader);
	glAttachShader(program_dim, fragment_shader_dim);
	glLinkProgram(program_dim);

	glGetProgramiv(program_dim, GL_LINK_STATUS, &linked);
	if (!linked) {
		char buf[1024];
		int len;
		log_info("Program link error\n");
		glGetProgramInfoLog(program_dim, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* プログラム(ルール)を作成する */
	program_rule = glCreateProgram();
	glAttachShader(program_rule, vertex_shader);
	glAttachShader(program_rule, fragment_shader_rule);
	glLinkProgram(program_rule);

	glGetProgramiv(program_rule, GL_LINK_STATUS, &linked);
	if (!linked) {
		char buf[1024];
		int len;
		log_info("Program link error\n");
		glGetProgramInfoLog(program_rule, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* プログラム(メルト)を作成する */
	program_melt = glCreateProgram();
	glAttachShader(program_melt, vertex_shader);
	glAttachShader(program_melt, fragment_shader_melt);
	glLinkProgram(program_melt);

	glGetProgramiv(program_melt, GL_LINK_STATUS, &linked);
	if (!linked) {
		char buf[1024];
		int len;
		log_info("Program link error\n");
		glGetProgramInfoLog(program_melt, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	/* シェーダのセットアップを行う(通常) */
	glUseProgram(program);
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	pos_loc = glGetAttribLocation(program, "a_position");
	glVertexAttribPointer((GLuint)pos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray((GLuint)pos_loc);
	tex_loc = glGetAttribLocation(program, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);
	alpha_loc = glGetAttribLocation(program, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);
	sampler_loc = glGetUniformLocation(program, "s_texture");
	glUniform1i(sampler_loc, 0);
	glGenBuffers(1, &index_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/* シェーダのセットアップを行う(DIM) */
	glUseProgram(program_dim);
	glGenVertexArrays(1, &vertex_array_dim);
	glBindVertexArray(vertex_array_dim);
	glGenBuffers(1, &vertex_buf_dim);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf_dim);
	pos_loc = glGetAttribLocation(program_dim, "a_position");
	glVertexAttribPointer((GLuint)pos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray((GLuint)pos_loc);
	tex_loc = glGetAttribLocation(program_dim, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);
	alpha_loc = glGetAttribLocation(program_dim, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);
	sampler_loc = glGetUniformLocation(program_dim, "s_texture");
	glUniform1i(sampler_loc, 0);
	glGenBuffers(1, &index_buf_dim);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf_dim);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/* シェーダのセットアップを行う(ルール) */
	glUseProgram(program_rule);
	glGenVertexArrays(1, &vertex_array_rule);
	glBindVertexArray(vertex_array_rule);
	glGenBuffers(1, &vertex_buf_rule);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf_rule);
	pos_loc = glGetAttribLocation(program_rule, "a_position");
	glVertexAttribPointer((GLuint)pos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray((GLuint)pos_loc);
	tex_loc = glGetAttribLocation(program_rule, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);
	alpha_loc = glGetAttribLocation(program_rule, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);
	sampler_loc = glGetUniformLocation(program_rule, "s_texture");
	glUniform1i(sampler_loc, 0);
	rule_loc = glGetUniformLocation(program_rule, "s_rule");
	glUniform1i(rule_loc, 1);
	glGenBuffers(1, &index_buf_rule);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf_rule);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/* シェーダのセットアップを行う(メルト) */
	glUseProgram(program_melt);
	glGenVertexArrays(1, &vertex_array_melt);
	glBindVertexArray(vertex_array_melt);
	glGenBuffers(1, &vertex_buf_melt);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf_melt);
	pos_loc = glGetAttribLocation(program_melt, "a_position");
	glVertexAttribPointer((GLuint)pos_loc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray((GLuint)pos_loc);
	tex_loc = glGetAttribLocation(program_melt, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);
	alpha_loc = glGetAttribLocation(program_melt, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const GLvoid *)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);
	sampler_loc = glGetUniformLocation(program_melt, "s_texture");
	glUniform1i(sampler_loc, 0);
	rule_loc = glGetUniformLocation(program_rule, "s_rule");
	glUniform1i(rule_loc, 1);
	glGenBuffers(1, &index_buf_melt);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf_melt);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	return true;
}

/*
 * Cleanup the Suika2's OpenGL rendering subsystem.
 *  - Note: On Emscripten, this will never be called
 */
void cleanup_opengl(void)
{
	if (fragment_shader_melt != 0)
		glDeleteShader(fragment_shader_melt);
	if (fragment_shader_rule != 0)
		glDeleteShader(fragment_shader_rule);
	if (fragment_shader != 0)
		glDeleteShader(fragment_shader);
	if (vertex_shader != 0)
		glDeleteShader(vertex_shader);
	if (program_melt != 0)
		glDeleteProgram(program_melt);
	if (program_rule != 0)
		glDeleteProgram(program_rule);
	if (program != 0)
		glDeleteProgram(program);
	if (vertex_array_melt != 0)
		glDeleteVertexArrays(1, &vertex_array_melt);
	if (vertex_array_rule != 0)
		glDeleteVertexArrays(1, &vertex_array_rule);
	if (vertex_array != 0)
		glDeleteVertexArrays(1, &vertex_array);
	if (vertex_buf_melt != 0)
		glDeleteBuffers(1, &vertex_buf_melt);
	if (vertex_buf_rule != 0)
		glDeleteBuffers(1, &vertex_buf_rule);
	if (vertex_buf != 0)
		glDeleteBuffers(1, &vertex_buf);
}

/*
 * Start a frame rendering.
 */
void opengl_start_rendering(void)
{
#ifndef USE_QT
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
}

/*
 * End a frame rendering.
 */
void opengl_end_rendering(void)
{
	glFlush();
}

/*
 * Texture manipulation:
 *  - "Texture" here is a GPU backend of an image
 *  - Suika2 abstracts modifications of textures by "lock/unlock" operations
 *  - However, OpenGL doesn't have a mechanism to lock pixels
 *  - Thus, we need to call glTexImage2D() to update entire texture for every "unlock" operations
 */

/*
 * Lock a texture.
 *  - This will just allocate memory for a texture management struct
 *  - We just use pixels of a frontend image for modification
 */
bool
opengl_lock_texture(
	int width,			/* IN: Image width */
	int height,			/* IN: Image height */
	pixel_t *pixels,		/* IN: Image pixels */
	pixel_t **locked_pixels,	/* OUT: Pixel pointer to modify image */
	void **texture)			/* OUT: Texture object */
{
	struct texture *tex;

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);

	assert(*locked_pixels == NULL);

	/*
	 * If a texture object for the image that uses "pixels" is not created yet.
	 * (In other words, the image was created by create_image(), but still
	 *  not drawn or cleared. This lazy initialization achieves a bit
	 *  optimization.)
	 */
	if (*texture == NULL) {
		/* Allocate memory for a texture struct. */
		tex = malloc(sizeof(struct texture));
		if (tex == NULL) {
			log_memory();
			return false;
		}

		tex->is_initialized = false;
		*texture = tex;
	}

	/*
	 * For image updates until unlock, we'll just use the area
	 * that "pixels" points to.
	 */
	*locked_pixels = pixels;

	return true;
}

/*
 * Unlock a texture.
 *  - This function uploads the contents that "pixels" points to,
 *    from CPU memory to GPU memory
 */
void
opengl_unlock_texture(
	int width,			/* IN: Image width */
	int height,			/* IN: Image height */
	pixel_t *pixels,		/* IN: Image pixels */
	pixel_t **locked_pixels,	/* IN/OUT: Pixel pointer to modify image (to be NULL) */
	void **texture)			/* IN: Texture object */
{
	struct texture *tex;

	UNUSED_PARAMETER(pixels);

	assert(*locked_pixels != NULL);

	tex = (struct texture *)*texture;

	/* If this is the first unlock. */
	if (!tex->is_initialized) {
		glGenTextures(1, &tex->id);
		tex->is_initialized = true;
	}

	/* Create or update an OpenGL texture. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, tex->id);
#ifdef EM
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, *locked_pixels);
	glActiveTexture(GL_TEXTURE0);

	/* Set NULL to "locked_pixels" to show it is not active. */
	*locked_pixels = NULL;
}

/*
 * Destroy a texture.
 */
void opengl_destroy_texture(void *texture)
{
	struct texture *tex;

	/* Destroy if a texture struct is allocated. */
	if (texture != NULL) {
		tex = (struct texture *)texture;

		/* Delete an OpenGL texture if it exists. */
		if (tex->is_initialized)
			glDeleteTextures(1, &tex->id);

		/* Free a texture struct. */
		free(tex);
	}
}

/*
 * 画面にイメージをレンダリングする
 */
void opengl_render_image(int dst_left, int dst_top,
			 struct image * RESTRICT src_image, int width,
			 int height, int src_left, int src_top, int alpha,
			 int bt)
{
	UNUSED_PARAMETER(bt);

	/* 描画の必要があるか判定する */
	if (alpha == 0 || width == 0 || height == 0)
		return;	/* 描画の必要がない */
	if (!clip_by_source(get_image_width(src_image),
			   get_image_height(src_image),
			   &width, &height, &dst_left, &dst_top, &src_left,
			   &src_top))
		return;	/* 描画範囲外 */
	if (!clip_by_dest(conf_window_width, conf_window_height, &width,
			 &height, &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	draw_elements(dst_left, dst_top, src_image, NULL, false, false,
		      width, height, src_left, src_top, alpha, bt);
}

/*
 * 画面にイメージを暗くレンダリングする
 */
void opengl_render_image_dim(int dst_left, int dst_top,
			     struct image * RESTRICT src_image, int width,
			     int height, int src_left, int src_top)
{
	/* 描画の必要があるか判定する */
	if (width == 0 || height == 0)
		return;	/* 描画の必要がない */
	if (!clip_by_source(get_image_width(src_image),
			   get_image_height(src_image),
			   &width, &height, &dst_left, &dst_top, &src_left,
			   &src_top))
		return;	/* 描画範囲外 */
	if (!clip_by_dest(conf_window_width, conf_window_height, &width,
			 &height, &dst_left, &dst_top, &src_left, &src_top))
		return;	/* 描画範囲外 */

	draw_elements(dst_left, dst_top, src_image, NULL, true, false,
		      width, height, src_left, src_top, 255, BLEND_FAST);
}

/*
 * 画面にイメージをルール付きでレンダリングする
 */
void opengl_render_image_rule(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold)
{
	draw_elements(0, 0, src_image, rule_image, false, false,
		      conf_window_width, conf_window_height,
		      0, 0, threshold, BLEND_FAST);

}

/*
 * 画面にイメージをルール付き(メルト)でレンダリングする
 */
void opengl_render_image_melt(struct image * RESTRICT src_image,
			      struct image * RESTRICT rule_image,
			      int threshold)
{
	draw_elements(0, 0, src_image, rule_image, false, true,
		      conf_window_width, conf_window_height,
		      0, 0, threshold, BLEND_FAST);

}

/* 画像を描画する */
static void draw_elements(int dst_left, int dst_top,
			  struct image * RESTRICT src_image,
			  struct image * RESTRICT rule_image,
			  bool is_dim, bool is_melt,
			  int width, int height,
			  int src_left, int src_top,
			  int alpha, int bt)
{
	GLfloat pos[24];
	struct texture *tex, *rule;
	float hw, hh, tw, th;

	/* struct textureを取得する */
	tex = get_texture_object(src_image);
	assert(tex != NULL);
	if (rule_image != NULL) {
		rule = get_texture_object(rule_image);
		assert(rule != NULL);
	} else {
		rule = NULL;
	}

	/* ウィンドウサイズの半分を求める */
	hw = (float)conf_window_width / 2.0f;
	hh = (float)conf_window_height / 2.0f;

	/* テキスチャサイズを求める */
	tw = (float)get_image_width(src_image);
	th = (float)get_image_height(src_image);

	/* 左上 */
	pos[0] = ((float)dst_left - hw) / hw;
	pos[1] = -((float)dst_top - hh) / hh;
	pos[2] = 0.0f;
	pos[3] = (float)src_left / tw;
	pos[4] = (float)src_top / th;
	pos[5] = (float)alpha / 255.0f;

	/* 右上 */
	pos[6] = ((float)dst_left + (float)width - hw) / hw;
	pos[7] = -((float)dst_top - hh) / hh;
	pos[8] = 0.0f;
	pos[9] = (float)(src_left + width) / tw;
	pos[10] = (float)(src_top) / th;
	pos[11] = (float)alpha / 255.0f;

	/* 左下 */
	pos[12] = ((float)dst_left - hw) / hw;
	pos[13] = -((float)dst_top + (float)height - hh) / hh;
	pos[14] = 0.0f;
	pos[15] = (float)src_left / tw;
	pos[16] = (float)(src_top + height) / th;
	pos[17] = (float)alpha / 255.0f;

	/* 右下 */
	pos[18] = ((float)dst_left + (float)width - hw) / hw;
	pos[19] = -((float)dst_top + (float)height - hh) / hh;
	pos[20] = 0.0f;
	pos[21] = (float)(src_left + width) / tw;
	pos[22] = (float)(src_top + height) / th;
	pos[23] = (float)alpha / 255.0f;

	/* シェーダを設定して頂点バッファに書き込む */
	if (rule_image == NULL) {
		if (!is_dim) {
			/* 通常のアルファブレンド */
			glUseProgram(program);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
		} else {
			/* DIMシェーダ */
			glUseProgram(program_dim);
			glBindVertexArray(vertex_array);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
		}
	} else if (!is_melt) {
		/* ルールシェーダ */
		glUseProgram(program_rule);
		glBindVertexArray(vertex_array_rule);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buf_rule);
	} else {
		/* メルトシェーダ */
		glUseProgram(program_melt);
		glBindVertexArray(vertex_array_melt);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buf_melt);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	/* テクスチャを選択する */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->id);
	if (rule_image != NULL) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, rule->id);
	}

	/* 透過を有効にする */
	glEnable(GL_BLEND);
	if (bt != BLEND_ADD) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glBlendFunc(GL_DST_ALPHA, GL_ONE);
	}

	/* 図形を描画する */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

#ifdef WIN
/*
 * 全画面表示のときのスクリーンオフセットを指定する
 */
void opengl_set_screen_offset(int x, int y)
{
	glViewport(x, y, conf_window_width, conf_window_height);
}
#endif
