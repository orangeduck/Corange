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
		
		