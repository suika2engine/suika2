/* -*- tab-width: 8; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2021, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2021-08-06 Created.
 */

#include "suika.h"
#include "glesrender.h"

#ifdef IOS
#define GL_SILENCE_DEPRECATION
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
GLuint vertex_buf;
GLuint index_buf;

static const char *s_vShaderStr =
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

static const char *s_fShaderStr =
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

struct texture {
	GLuint id;
	bool is_initialized;
};

#if 0
struct vertex {
	float x, y, z;
	float u, v;
	float alpha;
};
#endif

/*
 * OpenGLの初期化処理を行う
 */
bool init_opengl(void)
{
	const GLushort indices[] = {0, 1, 2, 3};
	GLint pos_loc, tex_loc, alpha_loc, sampler_loc;

	/* ビューポートを設定する */
	glViewport(0, 0, conf_window_width, conf_window_height);

	/* 頂点シェーダを作成する */
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &s_vShaderStr, NULL);
	glCompileShader(vertex_shader);

	/* フラグメントシェーダを作成する */
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &s_fShaderStr, NULL);
	glCompileShader(fragment_shader);

	/* プログラムを作成する */
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glUseProgram(program);

	/* シェーダのセットアップを行う */
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

	/* 頂点のインデックスを用意する */
	glGenBuffers(1, &index_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/* 透過を有効にする */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

/* OpenGLの終了処理を行う */
void cleanup_opengl(void)
{
	/* Emscriptenでは終了処理は呼び出されない */
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteProgram(program);
}

/* フレームのレンダリングを開始する */
void opengl_start_rendering(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

/* フレームのレンダリングを終了する */
void opengl_end_rendering(void)
{
	glFlush();
}

/* テクスチャをロックする */
bool opengl_lock_texture(int width, int height, pixel_t *pixels,
			 pixel_t **locked_pixels, void **texture)
{
	struct texture *tex;

	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);

	assert(*locked_pixels == NULL);

	/* テクスチャが作成されていない場合 */
	if (*texture == NULL) {
		/* texture構造体のメモリを確保する */
		tex = malloc(sizeof(struct texture));
		if (tex == NULL)
			return false;

		tex->is_initialized = false;
		*texture = tex;
	}

	/* ピクセルをロックする(特にコピーは行わない) */
	*locked_pixels = pixels;

	return true;
}

/* テクスチャをアンロックする */
void opengl_unlock_texture(int width, int height, pixel_t *pixels,
			   pixel_t **locked_pixels, void **texture)
{
	struct texture *tex;

	UNUSED_PARAMETER(pixels);

	assert(*locked_pixels != NULL);

	tex = (struct texture *)*texture;

	if (!tex->is_initialized) {
		glGenTextures(1, &tex->id);
		tex->is_initialized = true;
	}

	/* テクスチャを作成する */
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

	/* ピクセルをアンロックする */
	*locked_pixels = NULL;
}

/* テクスチャを破棄する */
void opengl_destroy_texture(void *texture)
{
	struct texture *tex;

	if (texture != NULL) {
		tex = (struct texture *)texture;
		if (tex->is_initialized)
			glDeleteTextures(1, &tex->id);
		free(tex);
	}
}

/* 画面にイメージをレンダリングする */
void opengl_render_image(int dst_left, int dst_top,
			 struct image * RESTRICT src_image, int width,
			 int height, int src_left, int src_top, int alpha,
			 int bt)
{
	GLfloat pos[24];
	struct texture *tex;
	float hw, hh, tw, th;

	UNUSED_PARAMETER(bt);

	/* struct textureを取得する */
	tex = get_texture_object(src_image);
	assert(tex != NULL);

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

	/* 頂点バッファに書き込む */
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	/* テクスチャを選択する */
	glBindTexture(GL_TEXTURE_2D, tex->id);

	/* 図形を描画する */
	if (width == 1 && height == 1) {
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, 0);
	} else if (width == 1) {
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT,
			       (const GLvoid *)(1 * sizeof(GLushort)));
	} else if (height == 1 && alpha < 255) {
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_SHORT, 0);
	} else {
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	}
}

/* 画面にイメージをマスク描画でレンダリングする */
void opengl_render_image_mask(int dst_left, int dst_top,
			      struct image * RESTRICT src_image, int width,
			      int height, int src_left, int src_top, int mask)
{
	int alpha;

	UNUSED_PARAMETER(mask);

	alpha = (int)((float)mask / 27.0f * 255.0f);

	opengl_render_image(dst_left, dst_top, src_image, width, height,
			    src_left, src_top, alpha, BLEND_NONE);
}

/* 画面をクリアする */
void opengl_render_clear(int left, int top, int width, int height,
			 pixel_t color)
{
	UNUSED_PARAMETER(left);
	UNUSED_PARAMETER(top);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(color);
}
