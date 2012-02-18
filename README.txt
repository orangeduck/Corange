	~~~~~~~~~~~~~~~~~~~~~~~~~
	|  Corange game engine  |
	~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Version 0.5.1
	
	Written in Pure C (Because it is beautiful)
	
	Uses SDL and OpenGL.
	
	:: Running ::
	
		Corange is a library, but to take a quick look at some of the things it can do look at the Demos.
	
	
	:: Compiling ::
	
		To compile on Windows you need MinGW and then you should be able to run "make" as usual.
		
		$ make

		To compile on Linux you need to install SDL1.2. Then you should run "make linux"
		
		$ sudo apt-get install libsdl1.2-dev
		$ make linux
		
		
	:: Features ::
		
		* Small, Simple, Powerful, Cross platform
		* Clean and easy Asset, UI, Entity management
		* Forward and Deferred renderers
		
		
	:: Demos ::
		
		renderers 	~ Shows off the various renderers with shaders, shadows, animation etc.
		lut_gen 	~ Tool for generating color correction LUTs from a photoshop curves file.
		metaballs 	~ Uses OpenCL/OpenGL stuff to do Metaball rendering.
		noise 		~ Feedback based noise pattern on screen using shader. Can generate tileable perlin noise in software.
		platformer 	~ Basic platforming game. Fairly well commented.
		sea 		~ Renders a sea-like surface, a pretty ship, and some collision detection.
		
		
	:: Using / Contributing ::
	
		This is still mainly a personal project and so there are going to be lots of bugs, unfinished features and messy bits of code. The engine is heavily WIP and subject to sweeping changes. It isn't really viable to use without also being part of the project development and in communication with me. Rather than a full game engine like Unity, Corange is more of a framework and gives you access to features at about the same level as XNA.
		
		Still, it is a great excuse to practise your C and I very much welcome help. If the project appeals to you here are a couple of quick things that might help get you started.
		
			* First take a look at the demos. These give a brief overview of how Corange can be used. The platformer demo is probably the most commented.
			
			* There is no documentation so your first port of call is the header files and your second is the c files. The code has very minimal comments but should be pretty clear most of the time.
			
			* Corange doesn't hide anything from you. OpenGL and SDL calls are in the namespace so you've got access to the basics. The corange_init and corange_finish functions are fairly short so it is even possible to not call them and only use the components you want.
			
			* Structs are typedefed without their pointer. This means if a function returns you a pointer to something you probably are responsible for deleting it. The reason for this is a personal choice but there are also quite a few data types which are passed by value on the stack (vectors, matrices, spheres, boxes). I didn't want the notion of these to get confused.
			
			* Some important parts of the engine are the asset, UI and entity managers. These basically let you access and store assets (models, textures - objects in the file system) and entities (lights, cameras, engine objects) and UI elements. They clean up memory on destruction and let you get pointers from all parts of the code.
			
			* Corange mangles the namespace pretty badly, taking names such as "error", "warning", "v2" and "image". It isn't a general purpose library. But I've still tried to decouple stuff so it should be possible to extract certain code if you need it.
			
			* More coming...
			
			
