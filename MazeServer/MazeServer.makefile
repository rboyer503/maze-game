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
Debug_Libraries=-Wl,--start-group -lboost_system -lboost_thread -lpthread -Wl,--end-group
Release_Libraries=-Wl,--start-group -lboost_system -lboost_thread -lpthread -Wl,--end-group

# Archives...
Debug_Archives=../x64/gccDebug/MazeShared.a
Release_Archives=../x64/gccRelease/MazeShared.a

# Preprocessor definitions...
Debug_Preprocessor_Definitions=-D _LINUX -D GCC_BUILD -D _DEBUG -D _CONSOLE 
Release_Preprocessor_Definitions=-D _LINUX -D GCC_BUILD -D NDEBUG -D _CONSOLE 

# Implictly linked object files...
Debug_Implicitly_Linked_Objects=
Release_Implicitly_Linked_Objects=

# Compiler flags...
Debug_Compiler_Flags=-std=c++11 -O0 -g 
Release_Compiler_Flags=-std=c++11 -O2 

# Builds all configurations for this project...
.PHONY: build_all_configurations
build_all_configurations: Debug Release 

# Builds the Debug configuration...
.PHONY: Debug
Debug: create_folders x64/gccDebug/Maze.o x64/gccDebug/MazeManager.o x64/gccDebug/MazeServer.o x64/gccDebug/MazeSession.o x64/gccDebug/AIAgent.o
	g++ x64/gccDebug/Maze.o x64/gccDebug/MazeManager.o x64/gccDebug/MazeServer.o x64/gccDebug/MazeSession.o x64/gccDebug/AIAgent.o $(Debug_Archives) $(Debug_Library_Path) $(Debug_Libraries) -Wl,-rpath=/usr/local/lib -o ../x64/gccDebug/MazeServer

# Compiles file Maze.cpp for the Debug configuration...
-include x64/gccDebug/Maze.d
x64/gccDebug/Maze.o: Maze.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c Maze.cpp $(Debug_Include_Path) -o x64/gccDebug/Maze.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM Maze.cpp $(Debug_Include_Path) > x64/gccDebug/Maze.d

# Compiles file MazeManager.cpp for the Debug configuration...
-include x64/gccDebug/MazeManager.d
x64/gccDebug/MazeManager.o: MazeManager.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c MazeManager.cpp $(Debug_Include_Path) -o x64/gccDebug/MazeManager.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM MazeManager.cpp $(Debug_Include_Path) > x64/gccDebug/MazeManager.d

# Compiles file MazeServer.cpp for the Debug configuration...
-include x64/gccDebug/MazeServer.d
x64/gccDebug/MazeServer.o: MazeServer.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c MazeServer.cpp $(Debug_Include_Path) -o x64/gccDebug/MazeServer.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM MazeServer.cpp $(Debug_Include_Path) > x64/gccDebug/MazeServer.d

# Compiles file MazeSession.cpp for the Debug configuration...
-include x64/gccDebug/MazeSession.d
x64/gccDebug/MazeSession.o: MazeSession.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c MazeSession.cpp $(Debug_Include_Path) -o x64/gccDebug/MazeSession.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM MazeSession.cpp $(Debug_Include_Path) > x64/gccDebug/MazeSession.d

# Compiles file AIAgent.cpp for the Debug configuration...
-include x64/gccDebug/AIAgent.d
x64/gccDebug/AIAgent.o: AIAgent.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c AIAgent.cpp $(Debug_Include_Path) -o x64/gccDebug/AIAgent.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM AIAgent.cpp $(Debug_Include_Path) > x64/gccDebug/AIAgent.d

# Builds the Release configuration...
.PHONY: Release
Release: create_folders x64/gccRelease/Maze.o x64/gccRelease/MazeManager.o x64/gccRelease/MazeServer.o x64/gccRelease/MazeSession.o x64/gccRelease/AIAgent.o
	g++ x64/gccRelease/Maze.o x64/gccRelease/MazeManager.o x64/gccRelease/MazeServer.o x64/gccRelease/MazeSession.o x64/gccRelease/AIAgent.o $(Release_Archives) $(Release_Library_Path) $(Release_Libraries) -Wl,-rpath=/usr/local/lib -o ../x64/gccRelease/MazeServer

# Compiles file Maze.cpp for the Release configuration...
-include x64/gccRelease/Maze.d
x64/gccRelease/Maze.o: Maze.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c Maze.cpp $(Release_Include_Path) -o x64/gccRelease/Maze.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM Maze.cpp $(Release_Include_Path) > x64/gccRelease/Maze.d

# Compiles file MazeManager.cpp for the Release configuration...
-include x64/gccRelease/MazeManager.d
x64/gccRelease/MazeManager.o: MazeManager.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c MazeManager.cpp $(Release_Include_Path) -o x64/gccRelease/MazeManager.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM MazeManager.cpp $(Release_Include_Path) > x64/gccRelease/MazeManager.d

# Compiles file MazeServer.cpp for the Release configuration...
-include x64/gccRelease/MazeServer.d
x64/gccRelease/MazeServer.o: MazeServer.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c MazeServer.cpp $(Release_Include_Path) -o x64/gccRelease/MazeServer.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM MazeServer.cpp $(Release_Include_Path) > x64/gccRelease/MazeServer.d

# Compiles file MazeSession.cpp for the Release configuration...
-include x64/gccRelease/MazeSession.d
x64/gccRelease/MazeSession.o: MazeSession.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c MazeSession.cpp $(Release_Include_Path) -o x64/gccRelease/MazeSession.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM MazeSession.cpp $(Release_Include_Path) > x64/gccRelease/MazeSession.d

# Compiles file AIAgent.cpp for the Release configuration...
-include x64/gccRelease/AIAgent.d
x64/gccRelease/AIAgent.o: AIAgent.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c AIAgent.cpp $(Release_Include_Path) -o x64/gccRelease/AIAgent.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM AIAgent.cpp $(Release_Include_Path) > x64/gccRelease/AIAgent.d

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
	rm -f ../x64/gccDebug/MazeServer
	rm -f x64/gccRelease/*.o
	rm -f x64/gccRelease/*.d
	rm -f ../x64/gccRelease/MazeServer

