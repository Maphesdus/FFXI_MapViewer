This is a slightly modified version of an old FFXI MapViewer program written in 2016 by a talented FINAL FANTASY XI player going by the username galkareeve, who appears to have abandoned his work. This program was written in C++ and uses the OpenGL graphics rendering framework. The following GitHub directory is galkareeve's original repository and contains multiple projects by him:

https://github.com/galkareeve/ffxi

I had originally forked his repository, but when I tried to push anything to the repo, GitHub complained about large file sizes, and so I chose to re-upload just the MapViewer project by itself in order to reduce the file size. The only changes I've made (so far) are adding in some #DEFINE statements so that Visual Studio would stop complaining about experimental functions, as well as changing the FFXI installation directory to match the normal default directory (galkareeve had originally installed his instance of FFXI into a non-standard directory, which he then hardcoded into his project, and I fixed this in my forked copy of his code). 

So now I’m able to get his source code to compile and run, but it just displays a blank white window without loading or displaying any models. I am actively looking for volunteers who know enough about C++ and OpenGL to be able to reverse engineer galkareeve’s code and help me figure out how to get his MapViewer project to function.

The variable for the FFXI installation directory is called "char ffxidir[512]" and is located in the FFXILandscapeMesh.cpp file, in case you need to change it on your own build.
