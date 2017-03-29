/*
    This file is part of Dash Graphics Library
    Copyright 2017 Benjamin Collins
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of this 
    software and associated documentation files (the "Software"), to deal in the Software 
    without restriction, including without limitation the rights to use, copy, modify, merge, 
    publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
    to whom the Software is furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all copies or 
    substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
    
*/
#include <png.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "dashgl.h"

/******************************************************************************/
/** Vector3 Utils                                                            **/
/******************************************************************************/

void vec3_subtract(vec3 a, vec3 b, vec3 v) {
	
	vec3 tmp;
	tmp[0] = a[0] - b[0];
	tmp[1] = a[1] - b[1];
	tmp[2] = a[2] - b[2];
	
	v[0] = tmp[0];
	v[1] = tmp[1];
	v[2] = tmp[2];
}

void vec3_cross_multiply(vec3 a, vec3 b, vec3 v) {
	
	vec3 tmp;

	tmp[0] = a[1]*b[2] - a[2]*b[1];
	tmp[1] = a[2]*b[0] - a[0]*b[2];
	tmp[2] = a[0]*b[1] - a[1]*b[0];

	v[0] = tmp[0];
	v[1] = tmp[1];
	v[2] = tmp[2];

}

void vec3_normalize(vec3 a, vec3 v) {

	float p;

	p = 0.0f;
	p += a[0] * a[0];
	p += a[1] * a[1];
	p += a[2] * a[2];
	
	p = 1.0f / (float)sqrt(p);
	
	v[0] = a[0] * p;
	v[1] = a[1] * p;
	v[2] = a[2] * p;

}

/******************************************************************************/
/** Shader Utils                                                             **/
/******************************************************************************/

void dash_print_log(GLuint object) {

	char *log;
	GLint log_length = 0;

	if(glIsShader(object)) {
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	} else if(glIsProgram(object)) {
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

GLuint dash_create_shader(const char *filename, GLenum type) {

	FILE *fp;
	int file_len;
	char *source;

	fp = fopen(filename, "rb");
	if(!fp) {
		fprintf(stderr, "Could not open %s for reading\n", filename);
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	source = (char*)malloc(file_len + 1);
	fread(source, file_len, 1, fp);
	fclose(fp);
	source[file_len] = '\0';

	const GLchar *sources[] = {
		#ifdef GL_ES_VERSION_2_0
		"#version 100\n", //OpenGL ES 2.0
		#else
		"#version 120\n", // OpenGL 2.1
		#endif
		source
	};

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 2, sources, NULL);
	glCompileShader(shader);

	free((void*)source);

	GLint compile_ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE) {
		fprintf(stderr, "%s: ", filename);
		dash_print_log(shader);
		glDeleteShader(shader);
		return 0;
	}

	return shader;

}

GLuint dash_create_program(const char *vertex, const char *fragment) {

	GLuint vs = dash_create_shader(vertex, GL_VERTEX_SHADER);
	GLuint fs = dash_create_shader(fragment, GL_FRAGMENT_SHADER);
	if(vs == 0 || fs == 0) {
		return 0;
	}
	
	GLint link_ok;
	
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if(!link_ok) {
		fprintf(stderr, "Program Link Error: ");
		dash_print_log(program);
		return 0;
	}
	
	return program;

}




GLuint dash_texture_load(const char *filename) {

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
        return;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

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
			printf("RGBA\n");
			data = (unsigned char*)malloc(4 * width * height);
		break;
	}

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return;
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

/******************************************************************************/
/** Matrix Utils                                                             **/
/******************************************************************************/


void mat4_identity(mat4 m) {
	
	m[M_00] = 1.0f;
	m[M_01] = 0.0f;
	m[M_02] = 0.0f;
	m[M_03] = 0.0f;
	m[M_10] = 0.0f;
	m[M_11] = 1.0f;
	m[M_12] = 0.0f;
	m[M_13] = 0.0f;
	m[M_20] = 0.0f;
	m[M_21] = 0.0f;
	m[M_22] = 1.0f;
	m[M_23] = 0.0f;
	m[M_30] = 0.0f;
	m[M_31] = 0.0f;
	m[M_32] = 0.0f;
	m[M_33] = 1.0f;

}

void mat4_copy(mat4 a, mat4 m) {
	
	m[M_00] = a[M_00];
	m[M_01] = a[M_01];
	m[M_02] = a[M_02];
	m[M_03] = a[M_03];
	m[M_10] = a[M_10];
	m[M_11] = a[M_11];
	m[M_12] = a[M_12];
	m[M_13] = a[M_13];
	m[M_20] = a[M_20];
	m[M_21] = a[M_21];
	m[M_22] = a[M_22];
	m[M_23] = a[M_23];
	m[M_30] = a[M_30];
	m[M_31] = a[M_31];
	m[M_32] = a[M_32];
	m[M_33] = a[M_33];

}

void mat4_translate(vec3 t, mat4 m) {

	m[M_00] = 1.0f;
	m[M_10] = 0.0f;
	m[M_20] = 0.0f;
	m[M_30] = 0.0f;
	
	m[M_01] = 0.0f;
	m[M_11] = 1.0f;
	m[M_21] = 0.0f;
	m[M_31] = 0.0f;
	
	m[M_02] = 0.0f;
	m[M_12] = 0.0f;
	m[M_22] = 1.0f;
	m[M_32] = 0.0f;
	
	m[M_03] = t[0];
	m[M_13] = t[1];
	m[M_23] = t[2];
	m[M_33] = 1.0f;

}

void mat4_rotate_x(float x, mat4 m) {

	m[M_00] = 1.0f;
	m[M_01] = 0.0f;
	m[M_02] = 0.0f;
	m[M_03] = 0.0f;
	m[M_10] = 0.0f;
	m[M_11] = cos(x);
	m[M_12] =-sin(x);
	m[M_13] = 0.0f;
	m[M_20] = 0.0f;
	m[M_21] = sin(x);
	m[M_22] = cos(x);
	m[M_23] = 0.0f;
	m[M_30] = 0.0f;
	m[M_31] = 0.0f;
	m[M_32] = 0.0f;
	m[M_33] = 1.0f;

}

void mat4_rotate_y(float y, mat4 m) {

	m[M_00] = cos(y);
	m[M_01] = 0.0f;
	m[M_02] = sin(y);
	m[M_03] = 0.0f;
	m[M_10] = 0.0f;
	m[M_11] = 1.0f;
	m[M_12] = 0.0f;
	m[M_13] = 0.0f;
	m[M_20] =-sin(y);
	m[M_21] = 0.0f;
	m[M_22] = cos(y);
	m[M_23] = 0.0f;
	m[M_30] = 0.0f;
	m[M_31] = 0.0f;
	m[M_32] = 0.0f;
	m[M_33] = 1.0f;

}

void mat4_rotate_z(float z, mat4 m) {

	m[M_00] = cos(z);
	m[M_01] =-sin(z);
	m[M_02] = 0.0f;
	m[M_03] = 0.0f;
	m[M_10] = sin(z);
	m[M_11] = cos(z);
	m[M_12] = 0.0f;
	m[M_13] = 0.0f;
	m[M_20] = 0.0f;
	m[M_21] = 0.0f;
	m[M_22] = 1.0f;
	m[M_23] = 0.0f;
	m[M_30] = 0.0f;
	m[M_31] = 0.0f;
	m[M_32] = 0.0f;
	m[M_33] = 1.0f;

}


void mat4_multiply(mat4 a, mat4 b, mat4 m) {

	mat4 tmp;

	tmp[M_00] = a[M_00]*b[M_00]+a[M_01]*b[M_10]+a[M_02]*b[M_20]+a[M_03]*b[M_30];
	tmp[M_01] = a[M_00]*b[M_01]+a[M_01]*b[M_11]+a[M_02]*b[M_21]+a[M_03]*b[M_31];
	tmp[M_02] = a[M_00]*b[M_02]+a[M_01]*b[M_12]+a[M_02]*b[M_22]+a[M_03]*b[M_32];
	tmp[M_03] = a[M_00]*b[M_03]+a[M_01]*b[M_13]+a[M_02]*b[M_23]+a[M_03]*b[M_33];
   
	tmp[M_10] = a[M_10]*b[M_00]+a[M_11]*b[M_10]+a[M_12]*b[M_20]+a[M_13]*b[M_30];
	tmp[M_11] = a[M_10]*b[M_01]+a[M_11]*b[M_11]+a[M_12]*b[M_21]+a[M_13]*b[M_31];
	tmp[M_12] = a[M_10]*b[M_02]+a[M_11]*b[M_12]+a[M_12]*b[M_22]+a[M_13]*b[M_32];
	tmp[M_13] = a[M_10]*b[M_03]+a[M_11]*b[M_13]+a[M_12]*b[M_23]+a[M_13]*b[M_33];
 
	tmp[M_20] = a[M_20]*b[M_00]+a[M_21]*b[M_10]+a[M_22]*b[M_20]+a[M_23]*b[M_30];
	tmp[M_21] = a[M_20]*b[M_01]+a[M_21]*b[M_11]+a[M_22]*b[M_21]+a[M_23]*b[M_31];
	tmp[M_22] = a[M_20]*b[M_02]+a[M_21]*b[M_12]+a[M_22]*b[M_22]+a[M_23]*b[M_32];
	tmp[M_23] = a[M_20]*b[M_03]+a[M_21]*b[M_13]+a[M_22]*b[M_23]+a[M_23]*b[M_33];
 
	tmp[M_30] = a[M_30]*b[M_00]+a[M_31]*b[M_10]+a[M_32]*b[M_20]+a[M_33]*b[M_30];
	tmp[M_31] = a[M_30]*b[M_01]+a[M_31]*b[M_11]+a[M_32]*b[M_21]+a[M_33]*b[M_31];
	tmp[M_32] = a[M_30]*b[M_02]+a[M_31]*b[M_12]+a[M_32]*b[M_22]+a[M_33]*b[M_32];
	tmp[M_33] = a[M_30]*b[M_03]+a[M_31]*b[M_13]+a[M_32]*b[M_23]+a[M_33]*b[M_33];

	mat4_copy(tmp, m);

}

void mat4_rotate(vec3 r, mat4 m) {

	mat4 rot_x, rot_y, rot_z;
	mat4_rotate_x(r[0], rot_x);
	mat4_rotate_y(r[1], rot_y);
	mat4_rotate_z(r[2], rot_z);

	mat4_multiply(rot_x, rot_y, m);
	mat4_multiply(m, rot_z, m);

}


void mat4_look_at(vec3 eye, vec3 center, vec3 up, mat4 m) {
	
	mat4 a;
	vec3 f, s, t;
	
	vec3_subtract(center, eye, f);
	vec3_normalize(f, f);

	vec3_cross_multiply(f, up, s);
	vec3_normalize(s, s);

	vec3_cross_multiply(s, f, t);

	m[0] = s[0];
	m[1] = t[0];
	m[2] =-f[0];
	m[3] = 0.0f;

	m[4] = s[1];
	m[5] = t[1];
	m[6] =-f[1];
	m[7] = 0.0f;

	m[8] = s[2];
	m[9] = t[2];
	m[10] = -f[2];
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;

	eye[0] = -eye[0];
	eye[1] = -eye[1];
	eye[2] = -eye[2];

	mat4_translate(eye, a);
	mat4_multiply(m, a, m);

}

void mat4_perspective(float y_fov, float aspect, float n, float f, mat4 m) {

	float const a = 1.f / (float) tan(y_fov / 2.f);

	m[0] = a / aspect;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = a;
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = -((f + n) / (f - n));
	m[11] = -1.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = -((2.0f * f * n) / (f - n));
	m[15] = 0.0f;

}

void mat4_orthographic(int left, int right, int top, int bottom, mat4 m) {
	
	if(left == right) {
		fprintf(stderr, "mat4_orthographic left cannot equal right\n");
		exit(1);
	}

	if(top == bottom) {
		fprintf(stderr, "mat4_orthographic top cannot equal bottom\n");
		exit(1);
	}

	float zNear = -0.1f;
	float zFar = 1.0f;
	float inv_z = 1.0f / (zFar - zNear);
	float inv_y = 1.0f / (top - bottom);
	float inv_x = 1.0f / (right - left);

    m[M_00] = 2.0f * inv_x;
    m[M_10] = 0.0f;
    m[M_20] = 0.0f;
    m[M_30] = 0.0f;

    m[M_01] = 0.0f;
    m[M_11] = 2.0f * inv_y;
    m[M_21] = 0.0f;
    m[M_31] = 0.0f;

    m[M_02] = 0.0f;
    m[M_12] = 0.0f;
    m[M_22] = -2.0f * inv_z;
    m[M_32] = 0.0f;

    m[M_03] = -(right + left)*inv_x;
    m[M_13] = -(top + bottom)*inv_y;
    m[M_23] = -(zFar + zNear)*inv_z;
    m[M_33] = 1.0f;

}

/******************************************************************************/
/** End Program	                                                             **/
/******************************************************************************/
