DTAUnpacker 1.0
---------------

Description:
	Use this program to extract the .DTA files from Hidden & Dangerous 2 and Mafia. Place
	DTAUnpacker.exe and tmp.dll in your Hidden & Dangerous 2, or Mafia directory.

	You need to run the program from the command-line, and provide it with 3 arguments:
	>DTAUnpacker.exe [DTA file] [key1] [key2]

		example:

		>DTAunpacker.exe Sounds.dta 0x8D2965CA 0x4FE85106

	You can obtain the list of keys for specific files by simply running the program without
	the 3 arguments. Note that the keys are hardcoded so they cannot be changed.

	The program only works with .DTA version ISD0. H&D2:SS uses ISD1, which is a different
	file format. Not all files are supported at the moment, but they will be in the future.


Special Thanks To:
	MassaSnygga, "Accessing Mafias DTA files", http://www.kamalook.de/Mafia/DTA.html
	Lars


Author:
	Jovan
						Please distribute the source code with the program.
								Let me know if you have any problems.
