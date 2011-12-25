	~~~~~~~~~~~~~~~~~~~~~~~~~
	|  Corange game engine  |
	~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Written in Pure C (Because it is beautiful)
	
	Depends on SDL and OpenGL.
	
	:: Running ::
	
		Corange is a library, but to take a quick look at some of the things it can do run any of the exe files in the demos folder.
	
	:: Compiling ::
	
		To compile on Windows you need MinGW and then you should be able to run "make" as usual.
		
		$ make

		To compile on Linux you need to install SDL1.2 and lua5.1. Then you should run "make linux"

		$ sudo apt-get install libsdl1.2-dev
		$ sudo apt-get install liblua5.1-dev
		$ make linux
		
	
	:: Main Features ::
		
		* Small, simple and powerful
		* Clean and easy Asset and Entity management
		* Forward, Deferred, Text renderers
	
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
			
			* Corange doesn't hide anything from you. OpenGL and SDL calls are in the namespace so you've got access to everything Corange does.
			
			* Structs are typedefed without their pointer. This means if a function returns you a pointer to something you probably are responsible for deleting it. The reason for this is a personal choice but there are quite a few data types which are passed by value on the stack (vectors, matrices). I didn't want the concept of these to get confused with "handle" values.
			
			* Some important parts of the engine are the asset and entity managers. These basically let you access and store assets (models, textures, objects in the file system) and entities (lights, cameras, engine objects). They also clean up memory on destruction and let you get pointers from all parts of the code.
			
			* More coming...
			
			
