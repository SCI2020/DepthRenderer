# DepthRendering

This is a convenient tool for rendering depth maps of given obj.

# Building

## Build Requirements

This project requires [glfw](https://github.com/glfw/glfw), [OpenCV](https://github.com/opencv/opencv), [glm](https://github.com/g-truc/glm) and [boost-program_options](https://www.boost.org/). We recommend installing 3rd-party packages with [vcpkg](https://github.com/Microsoft/vcpkg).

```
vcpkg install glfw3:x64-windows
vcpkg install libjpeg-turbo:x64-windows
```

## Build Procedure

### Visual Studio (2015) or later

```
cd <build_directory>
cmake -A x64 -G"Visual Studio 14 2015" \
-DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake {source_directory}
```
