
# BreakOut

A simple game which follow the [OGL tutorial](https://learnopengl.com/In-Practice/2D-Game/Breakout).


## How to run it

1. Install the submodule.

  Maybe some source file was included as the submodule 
  
  ```sh
  $ git clone github.com/godotc/breakout
  $ git submodule init
  $ git submodule update
  ```

2. Install the  `xmake`

  Which used as the build system, and package manager of this project, you don't need any other steps to deal with depdendencies.
  
  see: https://xmake.io/#/guide/installation

3. Install depencies, build and run

  ```sh
  xmake require
  xmake build breakout 
  xmake run breakout
  ```
