# Description
This repository contains an implementation for the LCSA-LENGTHS data structure. This is a twick, and engineer optimization of the [LCSA](https://doi.org/10.1145/2594408). 
# Installation Guide
First clone the repo:
```
 git clone https://github.com/elarielcl/LCSA-LENGTHS.git
 ```
 
This project is a CMake project. To build this project with some runnables you should do

```
cd ../..
mkdir build
cd build
cmake ..
cmake .. # Issue: second cmake necessary to compile sdsl
make
```

You can add an executable by writing your file in the `executables` directory and add its name to the `executables/CMakeLists.txt` file, this adds the necessary libraries for you:
```
set(project_EXECUTABLES
        <new_executable>
        main)
...
```

 ## Usage Example
 You can find one in the main.cpp executable.
 
 # Contact
 Any error, improvement or suggestion you can write me to `elarielcl` at Gmail. 
