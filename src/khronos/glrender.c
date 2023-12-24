/* -*- coding: utf-8; tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-06 Created.
 *  2023-09-05 Refactored and supported for Qt6.
 *  2023-12-14 Removed Apple OpenGL.
 */

#include "glrender.h"

/*
 * Include headers.
 */

/*
 * Android (OpenGL ES 3.0)
 */
#if defined(SUIKA_TARGET_ANDROID)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Emscripten (We use OpenGL ES 3.0)
 */
#if defined(SUIKA_TARGET_WASM)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

/*
 * Linux and POSIX (OpenGL 3.2)
 */
#if defined(SUIKA_TARGET_POSIX)
#include <GL/gl.h>
#include "glhelper.h"
#endif

/*
 * Qt (We use a wrapper for QOpenGLFunctions class)
 */
#if defined(USE_QT)
#include "glhelper.h"
#endif

/*
 * Pipeline types.
 */

enum {
	PIPELINE_NORMAL,
	PIPELINE_ADD,
	PIPELINE_DIM,
	PIPELINE_RULE,
	PIPELINE_MELT,
};

/*
 * This is our vertex format. (6 parameters, XYZUVA...)
 */
struct pseudo_vertex_info {
	/* 0 (V_POS) */		float x;
	/* 1 */			float y;
	/* 2 */			float z; /* 0 */
	/* 3 (V_TEX) */		float u;
	/* 4 */			float v;
	/* 5 (V_ALPHA) */	float alpha;
};
enum {
	V_POS_X = 0,
	V_POS_Y = 1,
	V_TEX_U = 3,
	V_TEX_V = 4,
	V_ALPHA = 5,
	V_SIZE = 6,
};

/*
 * The sole vertex shader that is shared between all fragment shaders.
 */
static GLuint vertex_shader = (GLuint)-1;

/*
 * Fragment shaders.
 */

/* The normal alpha blending. */
static GLuint fragment_shader_normal = (GLuint)-1;

/* The character dimming. (RGB 50%) */
static GLuint fragment_shader_dim = (GLuint)-1;

/* The rule shader. (1-bit universal transition) */
static GLuint fragment_shader_rule = (GLuint)-1;

/* The melt shader. (8-bit universal transition) */
static GLuint fragment_shader_melt = (GLuint)-1;

/*
 * Program per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint program_normal;

/* For the character dimming. (RGB 50%) */
static GLuint program_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint program_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint program_melt;

/*
 * VAO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vao_normal;

/* For the character dimming. (RGB 50%) */
static GLuint vao_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vao_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vao_melt;

/*
 * VBO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint vbo_normal;

/* For the character dimming. (RGB 50%) */
static GLuint vbo_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint vbo_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint vbo_melt;

/*
 * IBO per fragment shader.
 */

/* For the normal alpha blending. */
static GLuint ibo_normal;

/* For the character dimming. (RGB 50%) */
static GLuint ibo_dim;

/* For the rule shader. (1-bit universal transition) */
static GLuint ibo_rule;

/* For the melt shader. (8-bit universal transition) */
static GLuint ibo_melt;

/*
 * The vertex shader source.
 */

/* The source string. */
static const char *vertex_shader_src =
#if !defined(SUIKA_TARGET_WASM)
	"#version 100                 \n"
#endif
	"attribute vec4 a_position;   \n" /* FIXME: vec3? */
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
 * Fragment shader sources.
 */

/* The normal alpha blending shader. */
static const char *fragment_shader_src_normal =
#if !defined(SUIKA_TARGET_WASM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT) && defined(SUIKA_TARGET_POSIX)
	"#undef mediump                                      \n"
#endif
#if !defined(USE_QT)
	"precision mediump float;                            \n"
#endif
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
static const char *fragment_shader_src_dim =
#if !defined(SUIKA_TARGET_WASM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT) && defined(SUIKA_TARGET_POSIX)
	"#undef mediump                                      \n"
#endif
#if !defined(USE_QT)
	"precision mediump float;                            \n"
#endif
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
static const char *fragment_shader_src_rule =
#if !defined(SUIKA_TARGET_WASM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT) && defined(SUIKA_TARGET_POSIX)
	"#undef mediump                                      \n"
#endif
#if !defined(USE_QT)
	"precision mediump float;                            \n"
#endif
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
static const char *fragment_shader_src_melt =
#if !defined(SUIKA_ATERGET_WASM)
	"#version 100                                        \n"
#endif
#if defined(USE_QT) && defined(SUIKA_TARGET_POSIX)
	"#undef mediump                                      \n"
#endif
#if !defined(USE_QT)
	"precision mediump float;                            \n"
#endif
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

/* Indicates if the first rendering after re-init. */
static bool is_after_reinit;

/*
 * The following functions are defined in this file if we don't use Qt.
 * In the case we use Qt, they are defined in openglwidget.cpp because
 * VAO/VBO/IBO are abstracted in a very different way on the framework.
 */
bool setup_vertex_shader(const char **vshader_src, GLuint *vshader);
bool setup_fragment_shader(const char **fshader_src, GLuint vshader,
			   bool use_second_texture, GLuint *fshader,
			   GLuint *prog, GLuint *vao, GLuint *vbo,
			   GLuint *ibo);
void cleanup_vertex_shader(GLuint vshader);
void cleanup_fragment_shader(GLuint fshader, GLuint prog, GLuint vao,
			     GLuint vbo, GLuint ibo);

/*
 * Forward declaration.
 */
static void draw_elements(int dst_left,
			  int dst_top,
			  int dst_width,
			  int dst_height,
			  struct image *src_image,
			  struct image *rule_image,
			  int src_left,
			  int src_top,
			  int src_width,
			  int src_height,
			  int alpha,
			  int pipeline);
static void update_texture_if_needed(struct image *img);

/*
 * Initialize the Suika2's OpenGL rendering subsystem.
 */
bool init_opengl(void)
{
#ifdef SUIKA_TARGET_ANDROID
	cleanup_opengl();
#endif

	/* Set a viewport. */
	glViewport(0, 0, conf_window_width, conf_window_height);

	/* Setup a vertex shader. */
	if (!setup_vertex_shader(&vertex_shader_src, &vertex_shader))
		return false;

	/* Setup the fragment shader for normal alpha blending. */
	if (!setup_fragment_shader(&fragment_shader_src_normal,
				   vertex_shader,
				   false, /* no second texture */
				   &fragment_shader_normal,
				   &program_normal,
				   &vao_normal,
				   &vbo_normal,
				   &ibo_normal))
		return false;

	/* Setup the fragment shader for character dimming (RGB 50%). */
	if (!setup_fragment_shader(&fragment_shader_src_dim,
				   vertex_shader,
				   false, /* no second texture */
				   &fragment_shader_dim,
				   &program_dim,
				   &vao_dim,
				   &vbo_dim,
				   &ibo_dim))
		return false;

	/* Setup the fragment shader for rule (1-bit universal transition). */
	if (!setup_fragment_shader(&fragment_shader_src_rule,
				   vertex_shader,
				   true, /* use second texture */
				   &fragment_shader_rule,
				   &program_rule,
				   &vao_rule,
				   &vbo_rule,
				   &ibo_rule))
		return false;

	/* Setup the fragment shader for melt (8-bit universal transition). */
	if (!setup_fragment_shader(&fragment_shader_src_melt,
				   vertex_shader,
				   true, /* use second texture */
				   &fragment_shader_melt,
				   &program_melt,
				   &vao_melt,
				   &vbo_melt,
				   &ibo_melt))
		return false;

	is_after_reinit = true;

	return true;
}

#ifndef USE_QT

/*
 * Setup a vertex shader.
 */
bool
setup_vertex_shader(
	const char **vshader_src,	/* IN: A vertex shader source string. */
	GLuint *vshader)		/* OUT: A vertex shader ID. */
{
	char buf[1024];
	GLint compiled;
	int len;

	*vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(*vshader, 1, vshader_src, NULL);
	glCompileShader(*vshader);

	/* Check errors. */
	glGetShaderiv(*vshader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		log_info("Vertex shader compile error");
		glGetShaderInfoLog(*vshader, sizeof(buf), &len, &buf[0]);
		log_info("%s", buf);
		return false;
	}

	return true;
}

/*
 * Cleanup a vertex shader.
 */
void cleanup_vertex_shader(GLuint vshader)
{
	glDeleteShader(vshader);
}

/*
 * Setup a fragment shader, a program, a VAO, a VBO and an IBO.
 */
bool
setup_fragment_shader(
	const char **fshader_src,	/* IN: A fragment shader source string. */
	GLuint vshader,			/* IN: A vertex shader ID. */
	bool use_second_texture,	/* IN: Whether to use a second texture. */
	GLuint *fshader,		/* OUT: A fragment shader ID. */
	GLuint *prog,			/* OUT: A program ID. */
	GLuint *vao,			/* OUT: A VAO ID. */
	GLuint *vbo,			/* OUT: A VBO ID. */
	GLuint *ibo)			/* OUT: An IBO ID. */
{
	char err_msg[1024];
	GLint pos_loc, tex_loc, alpha_loc, sampler_loc, rule_loc;
	GLint is_succeeded;
	int err_len;

	const GLushort indices[] = {0, 1, 2, 3};

	/* Create a fragment shader. */
	*fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(*fshader, 1, fshader_src, NULL);
	glCompileShader(*fshader);

	/* Check errors. */
	glGetShaderiv(*fshader, GL_COMPILE_STATUS, &is_succeeded);
	if (!is_succeeded) {
		log_info("Fragment shader compile error");
		glGetShaderInfoLog(*fshader, sizeof(err_msg), &err_len, &err_msg[0]);
		log_info("%s", err_msg);
		return false;
	}

	/* Create a program. */
	*prog = glCreateProgram();
	glAttachShader(*prog, vshader);
	glAttachShader(*prog, *fshader);
	glLinkProgram(*prog);
	glGetProgramiv(*prog, GL_LINK_STATUS, &is_succeeded);
	if (!is_succeeded) {
		log_info("Program link error\n");
		glGetProgramInfoLog(*prog, sizeof(err_msg), &err_len, &err_msg[0]);
		log_info("%s", err_msg);
		return false;
	}
	glUseProgram(*prog);

	/* Create a VAO. */
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	/* Create a VBO. */
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	/* Set the vertex attibute for "a_position" (V_POS) in the vertex shader. */
	pos_loc = glGetAttribLocation(*prog, "a_position");
	glVertexAttribPointer((GLuint)pos_loc,
			      3,	/* (x, y, z) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_SIZE * sizeof(GLfloat),
			      (const GLvoid *)V_POS_X);
	glEnableVertexAttribArray((GLuint)pos_loc);

	/* Set the vertex attibute for "a_texCoord" (V_TEX) in the vertex shader. */
	tex_loc = glGetAttribLocation(*prog, "a_texCoord");
	glVertexAttribPointer((GLuint)tex_loc,
			      2,	/* (u, v) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_SIZE * sizeof(GLfloat),
			      (const GLvoid *)(V_TEX_U * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)tex_loc);

	/* Set the vertex attibute for "a_alpha" (V_ALPHA) in the vertex shader. */
	alpha_loc = glGetAttribLocation(*prog, "a_alpha");
	glVertexAttribPointer((GLuint)alpha_loc,
			      1,	/* (alpha) */
			      GL_FLOAT,
			      GL_FALSE,
			      V_SIZE * sizeof(GLfloat),
			      (const GLvoid *)(V_ALPHA * sizeof(GLfloat)));
	glEnableVertexAttribArray((GLuint)alpha_loc);

	/* Setup "s_texture" in a fragment shader. */
	sampler_loc = glGetUniformLocation(*prog, "s_texture");
	glUniform1i(sampler_loc, 0);

	/* Setup "s_rule" in a fragment shader if we use a second texture. */
	if (use_second_texture) {
		rule_loc = glGetUniformLocation(*prog, "s_rule");
		glUniform1i(rule_loc, 1);
	}

	/* Create an IBO. */
	glGenBuffers(1, ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);

	return true;
}

void
cleanup_fragment_shader(
	GLuint fshader,
	GLuint prog,
	GLuint vao,
	GLuint vbo,
	GLuint ibo)
{
	GLuint a[1];

	/* Delete a fragment shader. */
	glDeleteShader(fshader);

	/* Delete a program. */
	glDeleteProgram(prog);

	/* Delete a VAO. */
	a[0] = vao;
	glDeleteVertexArrays(1, (const GLuint *)&a);

	/* Delete a VBO. */
	a[0] = vbo;
	glDeleteBuffers(1, (const GLuint *)&a);

	/* Delete an IBO. */
	a[0] = ibo;
	glDeleteBuffers(1, (const GLuint *)&a);
}

#endif	/* ifndef USE_QT */

/*
 * Cleanup the Suika2's OpenGL rendering subsystem.
 *  - Note: On Emscripten, this will never be called
 */
void cleanup_opengl(void)
{
	if (fragment_shader_normal != (GLuint)-1) {
		cleanup_fragment_shader(fragment_shader_normal,
					program_normal,
					vao_normal,
					vbo_normal,
					ibo_normal);
		fragment_shader_normal = (GLuint)-1;
	}
	if (fragment_shader_dim != (GLuint)-1) {
		cleanup_fragment_shader(fragment_shader_dim,
					program_dim,
					vao_dim,
					vbo_dim,
					ibo_dim);
		fragment_shader_dim = (GLuint)-1;
	}
	if (fragment_shader_rule != (GLuint)-1) {
		cleanup_fragment_shader(fragment_shader_rule,
					program_rule,
					vao_rule,
					vbo_rule,
					ibo_rule);
		fragment_shader_rule = (GLuint)-1;
	}
	if (fragment_shader_melt != (GLuint)-1) {
		cleanup_fragment_shader(fragment_shader_melt,
					program_melt,
					vao_melt,
					vbo_melt,
					ibo_melt);
		fragment_shader_melt = (GLuint)-1;
	}
	if (vertex_shader != (GLuint)-1) {
		cleanup_vertex_shader(vertex_shader);
		vertex_shader = (GLuint)-1;
	}
}

/*
 * Start a frame rendering.
 */
void opengl_start_rendering(void)
{
#if !defined(USE_QT)
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
	is_after_reinit = false;
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
void opengl_notify_image_update(struct image *img)
{
	img->need_upload = true;
}

/*
 * Destroy a texture.
 */
void opengl_notify_image_free(struct image *img)
{
	GLuint id;

	id = (GLuint)(uintptr_t)img->texture - 1;

	/* FIXME: is_after_reinit */
	if (id != 0) {
		glDeleteTextures(1, &id);
		img->texture = NULL;
	}

	img->need_upload = false;
}

/*
 * 画面にイメージをレンダリングする
 */
void opengl_render_image_normal(int dst_left,
				int dst_top,
				int dst_width,
				int dst_height,
				struct image *src_image,
				int src_left,
				int src_top,
				int src_width,
				int src_height,
				int alpha)
{
	if (dst_width == -1)
		dst_width = src_image->width;
	if (dst_height == -1)
		dst_height = src_image->height;
	if (src_width == -1)
		src_width = src_image->width;
	if (src_height == -1)
		src_height = src_image->height;

	/* 描画の必要があるか判定する */
	if (alpha == 0 || dst_width == 0 || dst_height == 0)
		return;	/* 描画の必要がない */

	draw_elements(dst_left,
		      dst_top,
		      dst_width,
		      dst_height,
		      src_image,
		      NULL,
		      src_left,
		      src_top,
		      src_width,
		      src_height,
		      alpha,
		      PIPELINE_NORMAL);
}

/*
 * 画面にイメージをレンダリングする
 */
void opengl_render_image_add(int dst_left,
			     int dst_top,
			     int dst_width,
			     int dst_height,
			     struct image *src_image,
			     int src_left,
			     int src_top,
			     int src_width,
			     int src_height,
			     int alpha)
{
	if (dst_width == -1)
		dst_width = src_image->width;
	if (dst_height == -1)
		dst_height = src_image->height;
	if (src_width == -1)
		src_width = src_image->width;
	if (src_height == -1)
		src_height = src_image->height;

	/* 描画の必要があるか判定する */
	if (alpha == 0 || dst_width == 0 || dst_height == 0)
		return;	/* 描画の必要がない */

	draw_elements(dst_left,
		      dst_top,
		      dst_width,
		      dst_height,
		      src_image,
		      NULL,
		      src_left,
		      src_top,
		      src_width,
		      src_height,
		      alpha,
		      PIPELINE_ADD);
}

/*
 * 画面にイメージを暗くレンダリングする
 */
void opengl_render_image_dim(int dst_left,
			     int dst_top,
			     int dst_width,
			     int dst_height,
			     struct image *src_image,
			     int src_left,
			     int src_top,
			     int src_width,
			     int src_height,
			     int alpha)
{
	if (dst_width == -1)
		dst_width = src_image->width;
	if (dst_height == -1)
		dst_height = src_image->height;
	if (src_width == -1)
		src_width = src_image->width;
	if (src_height == -1)
		src_height = src_image->height;

	/* 描画の必要があるか判定する */
	if (alpha == 0 || dst_width == 0 || dst_height == 0)
		return;	/* 描画の必要がない */

	draw_elements(dst_left,
		      dst_top,
		      dst_width,
		      dst_height,
		      src_image,
		      NULL,
		      src_left,
		      src_top,
		      src_width,
		      src_height,
		      255,
		      PIPELINE_DIM);
}

/*
 * 画面にイメージをルール付きでレンダリングする
 */
void opengl_render_image_rule(struct image *src_image,
			      struct image *rule_image,
			      int threshold)
{
	draw_elements(0,
		      0,
		      conf_window_width,
		      conf_window_height,
		      src_image,
		      rule_image,
		      0,
		      0,
		      conf_window_width,
		      conf_window_height,
		      threshold,
		      PIPELINE_RULE);
}

/*
 * 画面にイメージをルール付き(メルト)でレンダリングする
 */
void opengl_render_image_melt(struct image *src_image,
			      struct image *rule_image,
			      int progress)
{
	draw_elements(0,
		      0,
		      conf_window_width,
		      conf_window_height,
		      src_image,
		      rule_image,
		      0,
		      0,
		      conf_window_width,
		      conf_window_height,
		      progress,
		      PIPELINE_MELT);
}

/* 画像を描画する */
static void draw_elements(int dst_left,
			  int dst_top,
			  int dst_width,
			  int dst_height,
			  struct image *src_image,
			  struct image *rule_image,
			  int src_left,
			  int src_top,
			  int src_width,
			  int src_height,
			  int alpha,
			  int pipeline)
{
	GLfloat pos[24];
	float hw, hh, tw, th;
	GLuint tex1, tex2;

	update_texture_if_needed(src_image);
	update_texture_if_needed(rule_image);

	/* テクスチャを取得する */
	tex1 = (GLuint)(intptr_t)src_image->texture - 1;
	assert(tex1 != 0);
	if (rule_image != NULL) {
		tex2 = (GLuint)(intptr_t)rule_image->texture - 1;
		assert(tex1 != 0);
	} else {
		tex2 = 0;
	}

	/* ウィンドウサイズの半分を求める */
	hw = (float)conf_window_width / 2.0f;
	hh = (float)conf_window_height / 2.0f;

	/* テキスチャサイズを求める */
	tw = (float)src_image->width;
	th = (float)src_image->height;

	/* 左上 */
	pos[0] = ((float)dst_left - hw) / hw;
	pos[1] = -((float)dst_top - hh) / hh;
	pos[2] = 0.0f;
	pos[3] = (float)src_left / tw;
	pos[4] = (float)src_top / th;
	pos[5] = (float)alpha / 255.0f;

	/* 右上 */
	pos[6] = ((float)dst_left + (float)dst_width - hw) / hw;
	pos[7] = -((float)dst_top - hh) / hh;
	pos[8] = 0.0f;
	pos[9] = (float)(src_left + src_width) / tw;
	pos[10] = (float)(src_top) / th;
	pos[11] = (float)alpha / 255.0f;

	/* 左下 */
	pos[12] = ((float)dst_left - hw) / hw;
	pos[13] = -((float)dst_top + (float)dst_height - hh) / hh;
	pos[14] = 0.0f;
	pos[15] = (float)src_left / tw;
	pos[16] = (float)(src_top + src_height) / th;
	pos[17] = (float)alpha / 255.0f;

	/* 右下 */
	pos[18] = ((float)dst_left + (float)dst_width - hw) / hw;
	pos[19] = -((float)dst_top + (float)dst_height - hh) / hh;
	pos[20] = 0.0f;
	pos[21] = (float)(src_left + src_width) / tw;
	pos[22] = (float)(src_top + src_height) / th;
	pos[23] = (float)alpha / 255.0f;

	/* シェーダを設定して頂点バッファに書き込む */
	switch (pipeline) {
	case PIPELINE_NORMAL:
		glUseProgram(program_normal);
		glBindVertexArray(vao_normal);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_normal);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case PIPELINE_ADD:
		glUseProgram(program_normal);
		glBindVertexArray(vao_normal);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_normal);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	case PIPELINE_DIM:
		glUseProgram(program_dim);
		glBindVertexArray(vao_dim);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_dim);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_dim);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case PIPELINE_RULE:
		glUseProgram(program_rule);
		glBindVertexArray(vao_rule);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_rule);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_rule);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case PIPELINE_MELT:
		glUseProgram(program_melt);
		glBindVertexArray(vao_melt);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_melt);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_melt);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	default:
		assert(0);
		break;
	}

	/* 頂点を転送する */
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	/* テクスチャを選択する */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex1);
	if (rule_image != NULL) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex2);
	}

	/* 図形を描画する */
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

static void update_texture_if_needed(struct image *img)
{
	GLuint id;

	if (img == NULL)
		return;
	if (!is_after_reinit && !img->need_upload)
		return;

	if (is_after_reinit || img->texture == NULL) {
		glGenTextures(1, &id);
		img->texture = (void *)(intptr_t)(id + 1);
	} else {
		id = (GLuint)(intptr_t)img->texture - 1;
	}

	/* Create or update an OpenGL texture. */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, id);
#ifdef SUIKA_TARGET_WASM
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);
	glActiveTexture(GL_TEXTURE0);

	img->need_upload = false;
}

/*
 * 全画面表示のときのスクリーンオフセットを指定する
 */
void opengl_set_screen(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}
