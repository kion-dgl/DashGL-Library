# DashGL

DashGL is basic vector and matrix manipulation library written in C. It is designed to be compatible with OpenGL and centers around the concept that everything is an array. A 2d vertor is a float array with a length of two. A 3d vector is a float array with a length of three. 4d vectors and quarternions are a float array with a length of four. And four by four matrices are represented with a float array of sixteen. All of the calculations that use these types are performed by utlizing the numbered elements of these arrays. The result is a small simple library that doesn't use malloc and doesn't need any conversion when working with OpenGL. DashGL is available under the MIT license.

### Documentation

The documentation is a work in progress and will continued to be updated as I implement more of the library. The library is divided into several different parts defined by a given prefix.

*Shader* - Pertains to compiling shaders, compiling programs, fetching errors, and loading images  
*vec2* - 2D vectors  
*vec3* - 3D vectors  
*vec4* - 4D Vectors  
*quat* - Quarternion's  
*mat4* - 4x4 Matrices  

### Tutorials

Tutorials for how to get started with the DashGL library are available on [DashGL.com](https://dashgl.com/). This includes three separate tutorials for a Brickout Clone, an Invaders Clone and an Astroids clone.

### Quick Start

As of yet, DashGL is not a finalized library which can be installed from a package manager. To use DashGL, you will need to clone this repository into your project. Build, and then link when compiling.

To install dependencies on Debian / Ubuntu:
```
$ sudo apt-get install libgtk-3-dev libegl1-mesa-dev libpng-dev
```

To install dependencies on Ret Hat / Fedora:
```
$ sudo yum install epel-release
$ sudo yum install gtk3-devel mesa-libGL-devel libepoxy-devel libpng-devel
```

To clone the and build the repository:
```
$ git clone git@github.com:kion-dgl/DashGL-Library.git DashGL
$ gcc -c -o DashGL/dashgl.o DashGL/dashgl.c -lGL -lepoxy -lpng
```

An overly simplified main file:

```
#include "DashGL/dashgl.h"

int main() {

}
```

Then to compile:
```
$ gcc main.c DashGL/dashgl.o -lGL -lepoxy -lpng
```
