# ImageProcess
A platform to test OpenGL fragment shader with image post-processing methods (single pass)

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
