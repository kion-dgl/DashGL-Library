/*
    This file is part of Dash Graphics Library
    Copyright 2017 Benjamin Collins

    Permission is hereby granted, free of charge, to any person obtaining 
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including 
    without limitation the rights to use, copy, modify, merge, publish, 
    distribute, sublicense, and/or sell copies of the Software, and to 
    permit persons to whom the Software is furnished to do so, subject to 
    the following conditions:

    The above copyright notice and this permission notice shall be included     
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    
*/

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#include "dashgl.h"

/**********************************************************************/
/** Shader Utilities                                                 **/	
/**********************************************************************/
	
void shader_print_log(GLuint object) {

	char *log;
	GLint log_length;

	if (glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if (glIsProgram(object)) {
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else {
		fprintf(stderr, "Not a shader or program\n");
		return;
	}

	log = (char*)malloc(log_length);

	if(glIsShader(object)) {
		glGetShaderInfoLog(object, log_length, NULL, log);
	} else if(glIsProgram(object)) {
		glGetProgramInfoLog(object, log_length, NULL, log);
	}

	fprintf(stderr, "%s\n", log);
	free(log);

}

GLuint shader_load_file(const char *filename, GLenum type) {

	FILE *fp;
	unsigned int file_len;
	char *source;
	GLuint shader;
	GLint compile_ok;

	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "Could not open %s for reading\n", filename);
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	source = malloc(file_len + 1);
	fread(source, file_len, sizeof(char), fp);
	fclose(fp);
	source[file_len] = '\0';
	
	const GLchar *sources[] = { source };

	shader = glCreateShader(type);
	glShaderSource(shader, 1, sources, NULL);
	glCompileShader(shader);
	
	free(source);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE) {
		fprintf(stderr, "Shader compile error: %s\n", filename);
		shader_print_log(shader);
		glDeleteShader(shader);
		return 0;
	}

	return shader;

}

GLuint shader_load_program(const char *vertex, const char *fragment) {

	GLint link_ok;
	GLuint program;

	GLuint vs = shader_load_file(vertex, GL_VERTEX_SHADER);
	GLuint fs = shader_load_file(fragment, GL_FRAGMENT_SHADER);

	if(vs == 0 || fs == 0) {
		return 0;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if(!link_ok) {
		fprintf(stderr, "Program Link Error: ");
		shader_print_log(program);
		return 0;
	}

	return program;

}

GLuint shader_load_texture(const char *filename) {

	FILE *fp;

	GLuint texture_id;
	png_structp png_ptr;
	png_infop info_ptr;
	int width, height, bit_depth;
	unsigned char *data;
	int color_type, num_passes, y, x, i;
	char header[8];
	png_bytep *rows;

	fp = fopen(filename, "rb");
	if(fp == NULL) {
		fprintf(stderr, "Could not open %s for reading\n", filename);
		exit(1);
	}

	if(fread(header, 1, 8, fp) <= 0) {
		fprintf(stderr, "Could not read png header\n");
		exit(1);
	}
	
	if (png_sig_cmp(header, 0, 8)) {
		fprintf(stderr, "%s is not a valid png file\n", filename);
		exit(1);
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		exit(1);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		exit(1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		exit(1);
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	num_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	switch(color_type) {
		case 0:
			printf("grayscale\n");
			exit(1);
		break;
		case PNG_COLOR_TYPE_RGB:
			bit_depth = GL_RGB;
			color_type = 3;
			data = (unsigned char*)malloc(3 * width * height);
		break;
		case 3:
			printf("pallete look up table\n");
			exit(1);
		break;
		case 4:
			printf("grayscale with alpha\n");
			exit(1);
		break;
		case PNG_COLOR_TYPE_RGBA:
			bit_depth = GL_RGBA;
			color_type = 4;
			data = (unsigned char*)malloc(4 * width * height);
		break;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		exit(1);
	}

	rows = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y = 0; y < height; y++) {
		rows[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));
	}

	png_read_image(png_ptr, rows);
	fclose(fp);

	i = 0;
	for(y = 0; y < height; y++) {
		memcpy((void*)&data[y*width*color_type], (void*)rows[y], width * color_type);
		png_free(png_ptr, rows[y]);
	}

	png_free(png_ptr, rows);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		bit_depth,
		width,
		height,
		0,
		bit_depth,
		GL_UNSIGNED_BYTE,
		data
	);
	free(data);
	
	return texture_id;

}


/**********************************************************************/
/** Vec2 Utilities                                                   **/	
/**********************************************************************/

void vec2_print(vec2 coord) {

	printf("vec2: %.2f, %.2f\n", coord[0], coord[1]);

}

/**********************************************************************/
/** Vec3 Utilities                                                   **/	
/**********************************************************************/

void vec3_copy(vec3 in, vec3 out) {

	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];

}


void vec3_subtract(vec3 a, vec3 b, vec3 out) {
	
	vec3 tmp;
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	tmp[2] = a[2] - b[2];
	
	out[0] = tmp[0];
	out[1] = tmp[1];
	out[2] = tmp[2];

}

void vec3_cross_multiply(vec3 a, vec3 b, vec3 out) {
	
	vec3 tmp;

	tmp[0] = a[1]*b[2] - a[2]*b[1];
	tmp[1] = a[2]*b[0] - a[0]*b[2];
	tmp[2] = a[0]*b[1] - a[1]*b[0];

	out[0] = tmp[0];
	out[1] = tmp[1];
	out[2] = tmp[2];

}

void vec3_normalize(vec3 in, vec3 out) {

	float p;

	p = 0.0f;
	p += in[0] * in[0];
	p += in[1] * in[1];
	p += in[2] * in[2];
	
	p = 1.0f / (float)sqrt(p);
	
	out[0] = in[0] * p;
	out[1] = in[1] * p;
	out[2] = in[2] * p;

}

/**********************************************************************/
/** Mat4 Utilities                                                   **/	
/**********************************************************************/


void mat4_identity(mat4 out) {

	out[M4_00] = 1.0f;
	out[M4_01] = 0.0f;
	out[M4_02] = 0.0f;
	out[M4_03] = 0.0f;
	out[M4_10] = 0.0f;
	out[M4_11] = 1.0f;
	out[M4_12] = 0.0f;
	out[M4_13] = 0.0f;
	out[M4_20] = 0.0f;
	out[M4_21] = 0.0f;
	out[M4_22] = 1.0f;
	out[M4_23] = 0.0f;
	out[M4_30] = 0.0f;
	out[M4_31] = 0.0f;
	out[M4_32] = 0.0f;
	out[M4_33] = 1.0f;

}

void mat4_copy(mat4 in, mat4 out) {

	out[M4_00] = in[M4_00];
	out[M4_01] = in[M4_01];
	out[M4_02] = in[M4_02];
	out[M4_03] = in[M4_03];
	out[M4_10] = in[M4_10];
	out[M4_11] = in[M4_11];
	out[M4_12] = in[M4_12];
	out[M4_13] = in[M4_13];
	out[M4_20] = in[M4_20];
	out[M4_21] = in[M4_21];
	out[M4_22] = in[M4_22];
	out[M4_23] = in[M4_23];
	out[M4_30] = in[M4_30];
	out[M4_31] = in[M4_31];
	out[M4_32] = in[M4_32];
	out[M4_33] = in[M4_33];

}

void mat4_rotate_x(float x, mat4 out) {

	out[M4_00] = 1.0f;
	out[M4_01] = 0.0f;
	out[M4_02] = 0.0f;
	out[M4_03] = 0.0f;
	out[M4_10] = 0.0f;
	out[M4_11] = cos(x);
	out[M4_12] =-sin(x);
	out[M4_13] = 0.0f;
	out[M4_20] = 0.0f;
	out[M4_21] = sin(x);
	out[M4_22] = cos(x);
	out[M4_23] = 0.0f;
	out[M4_30] = 0.0f;
	out[M4_31] = 0.0f;
	out[M4_32] = 0.0f;
	out[M4_33] = 1.0f;

}

void mat4_rotate_y(float y, mat4 out) {

	out[M4_00] = cos(y);
	out[M4_01] = 0.0f;
	out[M4_02] = sin(y);
	out[M4_03] = 0.0f;
	out[M4_10] = 0.0f;
	out[M4_11] = 1.0f;
	out[M4_12] = 0.0f;
	out[M4_13] = 0.0f;
	out[M4_20] =-sin(y);
	out[M4_21] = 0.0f;
	out[M4_22] = cos(y);
	out[M4_23] = 0.0f;
	out[M4_30] = 0.0f;
	out[M4_31] = 0.0f;
	out[M4_32] = 0.0f;
	out[M4_33] = 1.0f;

}

void mat4_rotate_z(float z, mat4 out) {

	out[M4_00] = cos(z);
	out[M4_01] =-sin(z);
	out[M4_02] = 0.0f;
	out[M4_03] = 0.0f;
	out[M4_10] = sin(z);
	out[M4_11] = cos(z);
	out[M4_12] = 0.0f;
	out[M4_13] = 0.0f;
	out[M4_20] = 0.0f;
	out[M4_21] = 0.0f;
	out[M4_22] = 1.0f;
	out[M4_23] = 0.0f;
	out[M4_30] = 0.0f;
	out[M4_31] = 0.0f;
	out[M4_32] = 0.0f;
	out[M4_33] = 1.0f;

}

void mat4_multiply(mat4 a, mat4 b, mat4 out) {

	mat4 tmp;

	tmp[M4_00] = a[M4_00]*b[M4_00]+a[M4_01]*b[M4_10]+a[M4_02]*b[M4_20]+a[M4_03]*b[M4_30];
	tmp[M4_01] = a[M4_00]*b[M4_01]+a[M4_01]*b[M4_11]+a[M4_02]*b[M4_21]+a[M4_03]*b[M4_31];
	tmp[M4_02] = a[M4_00]*b[M4_02]+a[M4_01]*b[M4_12]+a[M4_02]*b[M4_22]+a[M4_03]*b[M4_32];
	tmp[M4_03] = a[M4_00]*b[M4_03]+a[M4_01]*b[M4_13]+a[M4_02]*b[M4_23]+a[M4_03]*b[M4_33];
   
	tmp[M4_10] = a[M4_10]*b[M4_00]+a[M4_11]*b[M4_10]+a[M4_12]*b[M4_20]+a[M4_13]*b[M4_30];
	tmp[M4_11] = a[M4_10]*b[M4_01]+a[M4_11]*b[M4_11]+a[M4_12]*b[M4_21]+a[M4_13]*b[M4_31];
	tmp[M4_12] = a[M4_10]*b[M4_02]+a[M4_11]*b[M4_12]+a[M4_12]*b[M4_22]+a[M4_13]*b[M4_32];
	tmp[M4_13] = a[M4_10]*b[M4_03]+a[M4_11]*b[M4_13]+a[M4_12]*b[M4_23]+a[M4_13]*b[M4_33];
 
	tmp[M4_20] = a[M4_20]*b[M4_00]+a[M4_21]*b[M4_10]+a[M4_22]*b[M4_20]+a[M4_23]*b[M4_30];
	tmp[M4_21] = a[M4_20]*b[M4_01]+a[M4_21]*b[M4_11]+a[M4_22]*b[M4_21]+a[M4_23]*b[M4_31];
	tmp[M4_22] = a[M4_20]*b[M4_02]+a[M4_21]*b[M4_12]+a[M4_22]*b[M4_22]+a[M4_23]*b[M4_32];
	tmp[M4_23] = a[M4_20]*b[M4_03]+a[M4_21]*b[M4_13]+a[M4_22]*b[M4_23]+a[M4_23]*b[M4_33];
 
	tmp[M4_30] = a[M4_30]*b[M4_00]+a[M4_31]*b[M4_10]+a[M4_32]*b[M4_20]+a[M4_33]*b[M4_30];
	tmp[M4_31] = a[M4_30]*b[M4_01]+a[M4_31]*b[M4_11]+a[M4_32]*b[M4_21]+a[M4_33]*b[M4_31];
	tmp[M4_32] = a[M4_30]*b[M4_02]+a[M4_31]*b[M4_12]+a[M4_32]*b[M4_22]+a[M4_33]*b[M4_32];
	tmp[M4_33] = a[M4_30]*b[M4_03]+a[M4_31]*b[M4_13]+a[M4_32]*b[M4_23]+a[M4_33]*b[M4_33];

	mat4_copy(tmp, out);

}


void mat4_translate(vec3 pos, mat4 out) {

	out[M4_00] = 1.0f;
	out[M4_10] = 0.0f;
	out[M4_20] = 0.0f;
	out[M4_30] = 0.0f;
	
	out[M4_01] = 0.0f;
	out[M4_11] = 1.0f;
	out[M4_21] = 0.0f;
	out[M4_31] = 0.0f;
	
	out[M4_02] = 0.0f;
	out[M4_12] = 0.0f;
	out[M4_22] = 1.0f;
	out[M4_32] = 0.0f;
	
	out[M4_03] = pos[0];
	out[M4_13] = pos[1];
	out[M4_23] = pos[2];
	out[M4_33] = 1.0f;

}

void mat4_orthagonal(float width, float height, mat4 out) {
	
	float right, left, top, bottom;
	bottom = 0;
	top = height;
	left = 0;
	right = width;

	float zNear = -0.1f;
	float zFar = 1.0f;
	float inv_z = 1.0f / (zFar - zNear);
	float inv_y = 1.0f / (top - bottom);
	float inv_x = 1.0f / (right - left);

	out[M4_00] = 2.0f * inv_x;
	out[M4_10] = 0.0f;
	out[M4_20] = 0.0f;
	out[M4_30] = 0.0f;

	out[M4_01] = 0.0f;
	out[M4_11] = 2.0f * inv_y;
	out[M4_21] = 0.0f;
	out[M4_31] = 0.0f;

	out[M4_02] = 0.0f;
	out[M4_12] = 0.0f;
	out[M4_22] = -2.0f * inv_z;
	out[M4_32] = 0.0f;

	out[M4_03] = -(right + left)*inv_x;
	out[M4_13] = -(top + bottom)*inv_y;
	out[M4_23] = -(zFar + zNear)*inv_z;
	out[M4_33] = 1.0f;

}

void mat4_lookat(vec3 eye, vec3 center, vec3 up, mat4 out) {
	
	mat4 a;
	vec3 f, s, t;
	
	vec3_subtract(center, eye, f);
	vec3_normalize(f, f);

	vec3_cross_multiply(f, up, s);
	vec3_normalize(s, s);

	vec3_cross_multiply(s, f, t);

	out[M4_00] = s[0];
	out[M4_10] = t[0];
	out[M4_20] =-f[0];
	out[M4_30] = 0.0f;

	out[M4_01] = s[1];
	out[M4_11] = t[1];
	out[M4_21] =-f[1];
	out[M4_31] = 0.0f;

	out[M4_02] = s[2];
	out[M4_12] = t[2];
	out[M4_22] = -f[2];
	out[M4_32] = 0.0f;

	out[M4_03] = 0.0f;
	out[M4_13] = 0.0f;
	out[M4_23] = 0.0f;
	out[M4_33] = 1.0f;

	eye[0] = -eye[0];
	eye[1] = -eye[1];
	eye[2] = -eye[2];

	mat4_translate(eye, a);
	mat4_multiply(out, a, out);

}

void mat4_perspective(float y_fov, float aspect, float n, float f, mat4 out) {

	float const a = 1.f / (float) tan(y_fov / 2.f);

	out[M4_00] = a / aspect;
	out[M4_01] = 0.0f;
	out[M4_02] = 0.0f;
	out[M4_03] = 0.0f;

	out[M4_01] = 0.0f;
	out[M4_11] = a;
	out[M4_21] = 0.0f;
	out[M4_31] = 0.0f;

	out[M4_02] = 0.0f;
	out[M4_12] = 0.0f;
	out[M4_22] = -((f + n) / (f - n));
	out[M4_32] = -1.0f;

	out[M4_03] = 0.0f;
	out[M4_13] = 0.0f;
	out[M4_23] = -((2.0f * f * n) / (f - n));
	out[M4_33] = 0.0f;

}

/**********************************************************************/
/** End Library Definitions                                          **/	
/**********************************************************************/

