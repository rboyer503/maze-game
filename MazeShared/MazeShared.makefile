# Compiler flags...
CPP_COMPILER = g++
C_COMPILER = gcc

# Include paths...
Debug_Include_Path=
Release_Include_Path=

# Library paths...
Debug_Library_Path=
Release_Library_Path=

# Additional libraries...
Debug_Libraries=
Release_Libraries=

# Preprocessor definitions...
Debug_Preprocessor_Definitions=-D _LINUX -D GCC_BUILD 
Release_Preprocessor_Definitions=-D _LINUX -D GCC_BUILD 

# Implictly linked object files...
Debug_Implicitly_Linked_Objects=
Release_Implicitly_Linked_Objects=

# Compiler flags...
Debug_Compiler_Flags=-std=c++11 -O0 
Release_Compiler_Flags=-std=c++11 -O2 -g 

# Builds all configurations for this project...
.PHONY: build_all_configurations
build_all_configurations: Debug Release 

# Builds the Debug configuration...
.PHONY: Debug
Debug: create_folders x64/gccDebug/GameData.o x64/gccDebug/GameMessage.o 
	ar rvs ../x64/gccDebug/MazeShared.a x64/gccDebug/GameData.o x64/gccDebug/GameMessage.o

# Compiles file GameData.cpp for the Debug configuration...
-include x64/gccDebug/GameData.d
x64/gccDebug/GameData.o: GameData.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c GameData.cpp $(Debug_Include_Path) -o x64/gccDebug/GameData.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM GameData.cpp $(Debug_Include_Path) > x64/gccDebug/GameData.d

# Compiles file GameMessage.cpp for the Debug configuration...
-include x64/gccDebug/GameMessage.d
x64/gccDebug/GameMessage.o: GameMessage.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c GameMessage.cpp $(Debug_Include_Path) -o x64/gccDebug/GameMessage.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM GameMessage.cpp $(Debug_Include_Path) > x64/gccDebug/GameMessage.d

# Builds the Release configuration...
.PHONY: Release
Release: create_folders x64/gccRelease/GameData.o x64/gccRelease/GameMessage.o 
	ar rvs ../x64/gccRelease/MazeShared.a x64/gccRelease/GameData.o x64/gccRelease/GameMessage.o

# Compiles file GameData.cpp for the Release configuration...
-include x64/gccRelease/GameData.d
x64/gccRelease/GameData.o: GameData.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c GameData.cpp $(Release_Include_Path) -o x64/gccRelease/GameData.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM GameData.cpp $(Release_Include_Path) > x64/gccRelease/GameData.d

# Compiles file GameMessage.cpp for the Release configuration...
-include x64/gccRelease/GameMessage.d
x64/gccRelease/GameMessage.o: GameMessage.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c GameMessage.cpp $(Release_Include_Path) -o x64/gccRelease/GameMessage.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM GameMessage.cpp $(Release_Include_Path) > x64/gccRelease/GameMessage.d

# Creates the intermediate and output folders for each configuration...
.PHONY: create_folders
create_folders:
	mkdir -p x64/gccDebug
	mkdir -p ../x64/gccDebug
	mkdir -p x64/gccRelease
	mkdir -p ../x64/gccRelease

# Cleans intermediate and output files (objects, libraries, executables)...
.PHONY: clean
clean:
	rm -f x64/gccDebug/*.o
	rm -f x64/gccDebug/*.d
	rm -f ../x64/gccDebug/*.a
	rm -f x64/gccRelease/*.o
	rm -f x64/gccRelease/*.d
	rm -f ../x64/gccRelease/*.a

