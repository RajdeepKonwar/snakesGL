# snakesGL [![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FRajdeepKonwar%2FsnakesGL.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FRajdeepKonwar%2FsnakesGL?ref=badge_shield)
3D snakes game developed using C++, OpenGL and GLSL

### Compile Instructions
#### Windows
* Open `snakesGL.sln`
* Right-click Solution 'stockast' in the Solution Explorer and select `Retarget solution`
* Build

#### Linux
```
make
```
Type `make clean` to clean object file and executable.

### Run Instructions
#### Windows
Simply run from Visual Studio or double-click the executable created inside `x64\\{config}\snakesGL.exe`

By default, the program will try and utilize the maximum system threads available. In order to use a specific number of threads, set the environment vairable `OMP_NUM_THREADS` equal to the number of threads you want.

#### Linux
Set the number of threads to be used for computation,
```
export OMP_NUM_THREADS=number_of_threads
```
For example, `export OMP_NUM_THREADS=8`.
Then run the program
```
./stockast
```

## Blog
https://snakesgl.wordpress.com/

[![snakesGL YouTube Link](https://img.youtube.com/vi/DJgKYX8bxGo/0.jpg)](https://youtu.be/DJgKYX8bxGo)
