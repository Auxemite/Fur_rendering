# OM3D
<a id="readme-top"></a>
# 3D Modeling Software

<br />
<div align="center">
  <a href="https://github.com/Auxemite/om3D/">
    <img src="data/screenshot.png" alt="3D Realistic Fur"> <!-- width="80" height="80"> -->
  </a>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project

The goal of this project was to simulate realistic fur with shell rendering, using modern 3D optimisations and modern rendering techniques.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Built With

The project is in CPP and the interface has been developped with IMGUI/OPGL/GLFW.
* [![Cpp][Cpp.cpp]][Cpp-url]
* [![OpenGL][OP.GL]][OPGL-url]
* [![ImGUI][IM.GUI]][IMGUI-url]
* [![GLFW][GL.FW]][GLFW-url]

## Getting Started

This project has been developped on Windows10 and Linux and should work on both. (Not tested on MacOs)

### Prerequisites

`cmake 3.20` minimum, `C++17`, and `Opengl 4.5`

### Installation

1. Clone the repo
```sh
git clone https://github.com/Auxemite/om3D
```

2. Build
```sh
mkdir -p build/debug
cd build/debug
cmake ../..
make -j
```

3. Run
``` sh
./om3D
```

## Important Info

For a more efficient way of using DEAR IMGUI, we have created a directory in src/frontend called "imgui". This is the code of DEAR IMGUI and is NOT our code. We use this library only for some help on the creation of the app.

<!-- AUTHORS -->
## Authors
Alexandre Lamure
Gregoire Angerand
Ernest Bardon
Kael Facon

<!-- MARKDOWN LINKS & IMAGES -->
<!--5586a6-->
[OP.GL]: https://img.shields.io/badge/opengl-FFFFFF?logo=opengl&style=for-the-badge
[OPGL-url]: https://opengl.org/

[Cpp.cpp]: https://img.shields.io/badge/c++-00599C?logo=c%2B%2B&style=for-the-badge
[Cpp-url]: https://www.cppreference.com/

[IM.GUI]: https://img.shields.io/badge/IMGUI-151617?logo=imgui&style=for-the-badge&logoColor=white
[IMGUI-url]: https://github.com/ocornut/imgui

[GL.FW]: https://img.shields.io/badge/GLFW-ff9a29?logo=glfw&style=for-the-badge
[GLFW-url]: https://glfw.org/
