Corange game engine
===================
	
Version 0.5.1

Written in Pure C (Because it is beautiful)

Uses SDL and OpenGL.

Running
-------
	
Corange is a library, but to take a quick look at some of the things it does you can [Look at some of the Demos](http://www.youtube.com/watch?v=482GxqTWXtA).
	
	
Compiling
---------
	
To compile on Windows you need MinGW and then you should be able to run "make" as usual.

    make

To compile on Linux you need to install SDL1.2. Then you should run "make"

    sudo apt-get install libsdl1.2-dev
    make
		
There is a bug in some of the current linux SDL distributions which disables the buttons on window resize. Compile the latest SDL release from source to overcome this.


Overview
--------

* Small, Simple, Powerful, Cross platform
* Clean and easy Asset, UI, Entity management
* Forward and Deferred renderers


Demos
-----

I'm a graphics programmer by trade so apologies that most of the demos are graphical apps; they're just what I love!

* __renderers__ Shows off the various renderers with shaders, shadows, animation etc.
* __lut_gen__ Tool for generating color correction LUTs from a photoshop curves file.
* __metaballs__ Uses OpenCL/OpenGL interop to do Metaball rendering.
* __noise__ Feedback based noise pattern on screen using shader. Can generate tileable perlin noise in software.
* __platformer__ Basic platforming game. Fairly well commented.
* __sea__ Renders a sea-like surface, a ship, and some collision detection.
* __scotland__ Demonstrates terrain system.
* __tessellation__ Demo showing tessellation shaders in OpenGL 4.
		
	
FAQ
---
		
* __How is that pronounced?__

	Rhymes with Purple.

* __Why not C++?__
	
	There are plenty of C++ engines which do what I've done here and better. Pure C game engines on the other hand are much rarer. Corange provided me an outlet to practice my C skills and Data Oriented Design. Of course if you are just linking to it you can still program your game/executable using C++.
	
* __What stuff does it do?__

	I've used it as a platform for trying out all sorts of techniques and effects. These features are not out-of-the-box or plug-in-and-play, but if you are a developer who has knowledge of what they are, you should be able to utilize what I have written. Some are WIP or rough around the edges.
	
	Forward / Deferred / UI / Text Rendering. Multiple Lights. Post effects. SSAO. Shadow Mapping. Color Correction. Skeletal Animation. Inverse Kinematics. Collision Detection. OpenCL support. Audio Mixing. Asset / Entity / UI Management. Terrain. File loaders including .dds, .wav, .bmp, .obj, .smd. Maths and Geometry. And More...
	
* __Can I use this for 2D stuff?__
	
	Certainly. Though corange doesn't provide a 2D renderer for you. That you can write yourself. Believe it or not, making a generalized 2D renderer can be exceedingly complicated when you have to optimise for different sprites, tile sets, dynamic objects and all sorts of other effects. You're better off writing the rendering code application specific.
	
* __Can I contact you about something?__

	Yes - contact@theorangeduck.com
			
			
Using / Contributing
--------------------
	
This is still mainly a personal project and so there are going to be lots of bugs, unfinished features and messy bits of code. The engine is heavily WIP and subject to sweeping changes. It isn't really viable to use without also being part of the project development and in communication with me. Rather than a full game engine like Unity, Corange is more of a framework and gives you access to features at about the same level as XNA.
		
Saying that, it is a great excuse to practise your C and I very much welcome help. If the project appeals to you here are a couple of quick things that might help get you started.
		
* First take a look at the demos. These give a brief overview of how Corange can be used. The platformer demo is probably the most commented.

* There is no real documentation so your first port of call is the header files and your second is the c files. The code has very minimal comments but should be pretty clear most of the time.

* Corange doesn't hide anything from you. OpenGL and SDL calls are in the namespace so you've got access to the basics. The corange_init and corange_finish functions are fairly short so it is even possible to not call them and only use the components you want.

* Structs are typedefed without their pointer. The reason for this is a personal choice but there are also quite a few data types which are passed by value on the stack (vectors, matrices, spheres, boxes). I didn't want the notion of these to get confused.

* Some important parts of the engine are the asset, UI and entity managers. These basically let you access and store assets (models, textures - objects in the file system) and entities (lights, cameras, engine objects) and UI elements. They clean up memory on destruction and let you get pointers from all parts of the code.

* Corange mangles the namespace pretty badly, taking names such as "error", "warning", "v2" and "image". It isn't a general purpose library. But I've still tried to decouple stuff so it should be possible to extract certain code if you need it.

* More coming...

