TODO List
=========

Main Objectives
---------------

* All path management should be replaced with const char of size max path.
* Move all path objects to end of structs.
* Make sure nobody is storing assets when they need handles
* Document code properly
* Make multi material behaviour default.

To Finish Demos
---------------

* Sea Game - Add depth pass and foam effects? Walk around on moving deck?

Tweaks/Cleanup
--------------

* Optimise SDL_ReadLine to read more than one character at once.
* Convert skeleton stuff to quaternions - though not sure if I trust my quaternion code. Slerp them nicely.
* Remove all fixed function stuff inc builtins from all shaders.
