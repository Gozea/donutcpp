Wanted to try the donut.c but with a controllable camera.
Uses SFML and the Eigen library.

you have to compile your files with SFML.

![til](donut_demo.gif)

### Requirement

You will need SFML to be able to draw the window :

```sudo apt-get install libsfml-dev```

As well as Eigen for some algebra stuff :

```sudo apt-get install libeigen3-dev```

### Running

Compile with SFML with :

```g++ raycaster.o -o raycaster -lsfml-graphics -lsfml-window -lsfml-system```

Once compiled, you can execute it with :

```./raycaster```
