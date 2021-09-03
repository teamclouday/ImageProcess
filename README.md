# ImageProcess
A platform to try compute shader filters for image post-processing methods

Now supporting multiple compute shader image filters as a pipeline  
For example:  
```
[0] edge detection filter (compute)
[1] sharpen filter (compute)
[Final] output (vertex-fragment)
```
Orders can be modified

------

### Build

```bash
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j4
```

Or (if there's no `make`)
```powershell
cmake --build . --config Release
```

------

### Requirements

* C++ compiler supporting cpp11  
* GPU supporting OpenGL 4.5  

------

### Usage

* `R` to reload shader file  
* `F12` to toggle UI display  

------

### Filter Format
The following is an example of a input -> output filter with no processing

```glsl
#version 450 core

layout (local_size_x=32, local_size_y=32, local_size_z=1) in;

layout (rgba32f, binding=0) readonly  uniform image2D imageIn;
layout (rgba32f, binding=1) writeonly uniform image2D imageOut;

void main()
{
    ivec2 baseUV = ivec2(gl_GlobalInvocationID.xy);
    baseUV = clamp(baseUV, ivec2(0,0), imageSize(imageIn));
    vec3 imgColor = imageLoad(imageIn, baseUV).rgb;
    imageStore(imageOut, baseUV, vec4(imgColor, 1.0));
}
```

------

### Demo

<img src="assets/demo.png" width="800" alt="demo">
