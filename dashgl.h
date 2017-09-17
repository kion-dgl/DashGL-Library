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

#ifndef DASHGL_UTILS
#define DASHGL_UTILS
	
	/**********************************************************************/
	/** Shader Utilities                                                 **/	
	/**********************************************************************/
	
	void shader_print_log(GLuint object);
	GLuint shader_load_file(const char *filename, GLenum type);
	GLuint shader_load_program(const char *vertex, const char *fragment);
	GLuint shader_load_texture(const char *filename);
	
	/**********************************************************************/
	/** Vector2 Utilities                                                **/	
	/**********************************************************************/

	// Define Type

	typedef float vec2[2];
	
	/**********************************************************************/
	/** Vector3 Utilities                                                **/	
	/**********************************************************************/

	// Define Type

	typedef float vec3[3];

	void vec3_subtract(vec3 a, vec3 b, vec3 dst);
	void vec3_multiply(vec3 a, vec3 b, vec3 dst);
	void vec3_normalize(vec3 a, vec3 dst);
	
	/**********************************************************************/
	/** Matrix3 Utilities                                                **/	
	/**********************************************************************/

	// Define Type

	typedef float mat4[3];
	
	#define M3_00 0
	#define M3_01 1
	#define M3_02 2
	#define M3_10 3
	#define M3_11 4
	#define M3_12 5
	#define M3_20 6
	#define M3_21 7
	#define M3_22 8

	// Basic Utility

	void mat3_identity(mat3 dst);
	void mat3_copy(mat3 src, mat3 dst);
	void mat3_multiply(mat3 a, mat3 b, mat3 dst);
	
	// Transformation Matrices

	void mat3_scale(mat4 a, vec2 scl, mat3 dst);
	void mat3_translate(mat4 a, vec2 pos, mat3 dst);
	void mat3_rotate_x(mat3 a, float x, mat3 dst);
	void mat3_rotate_y(mat3 a, float y, mat3 dst);
	void mat3_rotate_xy(mat3 a, vec2 rot, mat3 dst);
	void mat3_transform(mat3 a, vec2 scl, vec2 pos, vec2 rot, mat3 dst);

	// Projection Matrices 

	void mat3_orthographic(int width, int height, mat3 dst);

	/**********************************************************************/
	/** Matrix4 Utilities                                                **/	
	/**********************************************************************/

	// Define Type

	typedef float mat4[16];

	// Constants

	#define M4_00 0
	#define M4_10 1
	#define M4_20 2
	#define M4_30 3
	#define M4_01 4
	#define M4_11 5
	#define M4_21 6
	#define M4_31 7
	#define M4_02 8
	#define M4_12 9
	#define M4_22 10
	#define M4_32 11
	#define M4_03 12
	#define M4_13 13
	#define M4_23 14
	#define M4_33 15

	// Basic Utility

	void mat4_identity(mat4 dst);
	void mat4_copy(mat4 src, mat4 dst);
	void mat4_multiply(mat4 a, mat4 b, mat4 dst);
	
	// Transformation Matrices

	void mat4_scale(mat4 a, vec3 scl, mat4 dst);
	void mat4_translate(mat4 a, vec3 pos, mat4 dst);
	void mat4_rotate_x(mat4 a, float x, mat4 dst);
	void mat4_rotate_y(mat4 a, float y, mat4 dst);
	void mat4_rotate_z(mat4 a, float z, mat4 dst);
	void mat4_rotate_xyz(mat4 a, vec3 rot, mat4 dst);
	void mat4_transform(mat4 a, vec3 scl, vec3 pos, vec3 rot, mat4 dst);

	// Projection Matrices 

	void mat4_lookat(vec3 eye, vec3 center, vec3 up, mat4 dst);
	void mat4_perspective(float y_fov, float aspect, float near, float far, mat4 dst);
	void mat4_orthographic(float left, float right, float top, float bottom, mat4 dst);

	/**********************************************************************/
	/** End Library Definitions                                          **/	
	/**********************************************************************/

#endif
