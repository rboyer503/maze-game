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
Debug_Libraries=-Wl,--start-group -lboost_system -lboost_thread -lpthread -lncurses -Wl,--end-group
Debug_Libraries=-Wl,--start-group -lboost_system -lboost_thread -lncurses -Wl,--end-group
Release_Libraries=-Wl,--start-group -lboost_system -lboost_thread -lpthread -lncurses -Wl,--end-group

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
Debug_Compiler_Flags=-std=c++11 -O0 -g -pthread
Release_Compiler_Flags=-std=c++11 -O2 

# Builds all configurations for this project...
.PHONY: build_all_configurations
build_all_configurations: Debug Release 

# Builds the Debug configuration...
.PHONY: Debug
Debug: create_folders x64/gccDebug/ClientManager.o x64/gccDebug/LinuxTerminal.o x64/gccDebug/MazeClient.o
	g++ -pthread x64/gccDebug/ClientManager.o x64/gccDebug/LinuxTerminal.o x64/gccDebug/MazeClient.o $(Debug_Archives) $(Debug_Library_Path) $(Debug_Libraries) -Wl,-rpath=/usr/local/lib -o ../x64/gccDebug/MazeClient

# Compiles file ClientManager.cpp for the Debug configuration...
-include x64/gccDebug/ClientManager.d
x64/gccDebug/ClientManager.o: ClientManager.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c ClientManager.cpp $(Debug_Include_Path) -o x64/gccDebug/ClientManager.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM ClientManager.cpp $(Debug_Include_Path) > x64/gccDebug/ClientManager.d

# Compiles file LinuxTerminal.cpp for the Debug configuration...
-include x64/gccDebug/LinuxTerminal.d
x64/gccDebug/LinuxTerminal.o: LinuxTerminal.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c LinuxTerminal.cpp $(Debug_Include_Path) -o x64/gccDebug/LinuxTerminal.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM LinuxTerminal.cpp $(Debug_Include_Path) > x64/gccDebug/LinuxTerminal.d

# Compiles file MazeClient.cpp for the Debug configuration...
-include x64/gccDebug/MazeClient.d
x64/gccDebug/MazeClient.o: MazeClient.cpp
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -c MazeClient.cpp $(Debug_Include_Path) -o x64/gccDebug/MazeClient.o
	$(CPP_COMPILER) $(Debug_Preprocessor_Definitions) $(Debug_Compiler_Flags) -MM MazeClient.cpp $(Debug_Include_Path) > x64/gccDebug/MazeClient.d

# Builds the Release configuration...
.PHONY: Release
Release: create_folders x64/gccRelease/ClientManager.o x64/gccRelease/LinuxTerminal.o x64/gccRelease/MazeClient.o
	g++ x64/gccRelease/ClientManager.o x64/gccRelease/LinuxTerminal.o x64/gccRelease/MazeClient.o $(Release_Archives) $(Release_Library_Path) $(Release_Libraries) -Wl,-rpath=/usr/local/lib -o ../x64/gccRelease/MazeClient

# Compiles file ClientManager.cpp for the Release configuration...
-include x64/gccRelease/ClientManager.d
x64/gccRelease/ClientManager.o: ClientManager.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c ClientManager.cpp $(Release_Include_Path) -o x64/gccRelease/ClientManager.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM ClientManager.cpp $(Release_Include_Path) > x64/gccRelease/ClientManager.d

# Compiles file LinuxTerminal.cpp for the Release configuration...
-include x64/gccRelease/LinuxTerminal.d
x64/gccRelease/LinuxTerminal.o: LinuxTerminal.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c LinuxTerminal.cpp $(Release_Include_Path) -o x64/gccRelease/LinuxTerminal.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM LinuxTerminal.cpp $(Release_Include_Path) > x64/gccRelease/LinuxTerminal.d

# Compiles file MazeClient.cpp for the Release configuration...
-include x64/gccRelease/MazeClient.d
x64/gccRelease/MazeClient.o: MazeClient.cpp
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -c MazeClient.cpp $(Release_Include_Path) -o x64/gccRelease/MazeClient.o
	$(CPP_COMPILER) $(Release_Preprocessor_Definitions) $(Release_Compiler_Flags) -MM MazeClient.cpp $(Release_Include_Path) > x64/gccRelease/MazeClient.d

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
	rm -f ../x64/gccDebug/MazeClient
	rm -f x64/gccRelease/*.o
	rm -f x64/gccRelease/*.d
	rm -f ../x64/gccRelease/MazeClient

