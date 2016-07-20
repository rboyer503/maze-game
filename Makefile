# Define top-level rules.
.PHONY: all
all:
	@echo "Please specify 'debug' or 'release'."
	
.PHONY: debug
debug: VARIANT = debug
debug: MazeServer MazeClient MazeShared 

.PHONY: release
release: VARIANT = release
release: MazeServer MazeClient MazeShared


# Define project rules.
.PHONY: MazeServer
MazeServer: MazeShared
	@echo
	@echo "[Building MazeServer Project...]"
	make --directory="MazeServer/" --file=MazeServer.makefile $(VARIANT)

.PHONY: MazeClient
MazeClient: MazeShared
	@echo
	@echo "[Building MazeClient Project...]"
	make --directory="MazeClient/" --file=MazeClient.makefile $(VARIANT)

.PHONY: MazeShared
MazeShared:
	@echo
	@echo "[Building MazeShared Project...]" 
	make --directory="MazeShared/" --file=MazeShared.makefile $(VARIANT)


# Clean all projects.
.PHONY: clean
clean:
	make --directory="MazeServer/" --file=MazeServer.makefile clean
	make --directory="MazeClient/" --file=MazeClient.makefile clean
	make --directory="MazeShared/" --file=MazeShared.makefile clean

