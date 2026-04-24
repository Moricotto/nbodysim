# nbodysim

N body simulation in opengl and with c++.

## Building
To build the project you will need to have `cmake` installed. Consult the following website for details on how to do this: https://cmake.org/cmake/help/latest/command/install.html. 

1. Create a build directory
```shell
mkdir build;
```

2. Run cmake.

```shell
cmake -B build -S .
```

3. Compile the library
```shell
cmake --build build
```

## Running

After building you should be able to run the project depending on your os.

**On Windows**:
```shell
.\build\nbody.exe
```
**On Linux or Mac**:
```shell
./build/nbody
```

### Note on VSCode.
For those coding in vscode you will need to have the extension [CMake Tool](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) on top of the c/c++ extension. 

If you do have the extension you should delete the `build` folder as vscode may used different options than the default one that cmake habitually uses and will error out.

After deleting the folder, using the command pallete(`Ctrl+Maj+P`), try to run `CMake: Configure` and it should build the project for you.

Then to run the project you can either run via the command pallete `CMake: Run Without Debugging` or with `Ctrl+Shift+F5`  or with the button of the screen next to the build button.

