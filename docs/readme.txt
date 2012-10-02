modshroom 0.55
2.10.2012
written by cce

Usage: modshroom.exe input.mod [output.sho] [tempo] [author]
Note: No spaces allowed in the author field. The song name is deduced from the module title.

Converts 4 channel amiga modules to Mario Paint .sho format. The converter uses only the first 3 channels, so the fourth one is ignored. You can use the bundled template.mod to make your song, since it has already the samples tuned (kinda) and set up. Effects are not supported.

Mario Paint only supports the following notes: B-4, C-4, D-4, E-4, F-4, G-4, A-4, B-4, C-5, D-5, E-5, F-5 and G-5. If an invalid note is used the converter spits out an error and ignores the note. The maximum length of a mario paint song is 96 rows, and there is currently no way to make shorter songs with modshroom. The song tempo must be defined by hand on the command line, or the song is exported with the default tempo of 150.

The instruments "conga" and "open hihat" are actually just different pitches of the same instrument, but you shouldn't worry about that.

I also chose to bundle the Shroomtool & Shroomplayer with this package since no-one has them anyway. If you don't have them, get them from http://www.lofibucket.com/download/shroomtools.zip