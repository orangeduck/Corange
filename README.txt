	~~~~~~~~~~~~~~~~~~~~~~~~~
	|  Corange game engine  |
	~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Written in Pure C (Because it is beautiful)
	
	Depends on SDL and OpenGL.
	
	:: Running ::
	
		To run a game call corange from the command line with the first arg being the name of the game to play. For example if you wish to play the "cello" demo run the command:
			
			$ corange cello
			
		Otherwise it will default to the blank game called "empty" which will just display a blank screen.
	
		You can also run any of the demo .bat files
	
	:: Compiling ::
	
		To compile on Windows you need MinGW and then you should be able to run "make" as usual.
		To compile on Unix you need probably need some changes to the Makefile but it should be pretty straight forward.
	
	:: Main Features ::
		
		* Small, simple and powerful
		* Clean and easy Asset and Entity management
		* Forward, Deferred, Text renderers
		* Dynamically loaded game modules
	
	:: Other Things It does ::
	
		* Color correction, LUT generation, splines, photoshop curves
		* LUA scripting
		* Perlin noise
		* OpenCL support
		* Basic UI stuff
		* Loaders for OBJ, SMD, DDS, TGA, BMP and others
		* Skeletal Animation
		
	:: Using / Contributing ::
	
		This is still mainly a personal project and so there are going to be lots of bugs, unfinished features and messy bits of code. The engine is heavily WIP and subject to sweeping changes. It isn't really viable to use without also being part of the project development and in communication with me.
		
		Still, I very much welcome help, and if the project appeals to you here are a couple of quick things that might help get you started.
		
			* First take a look at the demos in the games folder. This gives a brief overview of how Corange can be used.
		
			* There is no documentation so your first port of call is the header files. The code also has very minimal comments. If you want to know how to do something, or if it is supported, research how it is done in general then see if Corange has anything that looks like it might help.
			
			* When extending with a game corange doesn't hide anything from you. OpenGL calls are in the namespace so you've got access to everything Corange does.
			
			* Structs are typedefed without their pointer. This means if a function returns you a pointer to something you probably are responsible for deleting it. The reason for this is a personal choice but there are quite a few data types which are passed by value on the stack (vectors, matricies). I didn't want the concept of these to get confused with "handle" values.
			
			* Some important parts of the engine are the asset and entity managers. These basically let you access and store assets (models, textures, objects in the file system) and entities (lights, cameras, engine objects). They also clean up memory on destruction and let you get pointers from all parts of the code.
			
			* 
			
			