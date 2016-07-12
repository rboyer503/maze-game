# Builds all the projects in the solution...
.PHONY: all_projects
all_projects: MazeServer MazeClient MazeShared 

# Builds project 'MazeServer'...
.PHONY: MazeServer
MazeServer: MazeShared
	make --directory="MazeServer/" --file=MazeServer.makefile

# Builds project 'MazeClient'...
.PHONY: MazeClient
MazeClient: MazeShared
	make --directory="MazeClient/" --file=MazeClient.makefile

# Builds project 'MazeShared'...
.PHONY: MazeShared
MazeShared: 
	make --directory="MazeShared/" --file=MazeShared.makefile

# Cleans all projects...
.PHONY: clean
clean:
	make --directory="MazeServer/" --file=MazeServer.makefile clean
	make --directory="MazeClient/" --file=MazeClient.makefile clean
	make --directory="MazeShared/" --file=MazeShared.makefile clean

