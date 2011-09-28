	~~~~~~~~~~~~~~~~~~~~~~~~~
	|  Corange game engine  |
	~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Written in Pure C
		(Why? Because it is beautiful)
	
	With SDL and OpenGL.
	
	:: Running ::
	
		To run a game call corange from the command line with the first arg being the name of the game to play. For example if you wish to play the "cello" demo run the command:
			
			$ corange cello
			
		Otherwise it will default to the blank game called "empty" which will just display a blank screen.
	
		You can also run any of the demo .bat files
	
	:: Features ::
	
		* Clean and simple asset management
		* Deferred and forward renderers
		* Text renderer
		* LUA scripting
		* Dynamically loaded game modules
		* Loads obj files and dds files
		* Small, Simple and powerful
	
	
	:: Compiling ::
	
		To compile on Windows you need MinGW with gcc and from that it should just compile
		To compile on Unix you need probably lots of things including a new Makefile and changes to the code.