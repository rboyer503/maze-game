# Adapted from http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/.

# Initialize variables.
CXXFLAGS := -std=c++11
SYMBOLS := -D _LINUX -D GCC_BUILD
LIB_NAME := MazeShared.a
DEBUG_OBJDIR := x64/gccDebug
RELEASE_OBJDIR := x64/gccRelease
LIBRARIES := -Wl,--start-group -lboost_system -lboost_thread -lpthread -lncurses -Wl,--end-group
BINARY_NAME := MazeClient


# Define top-level rules.
.PHONY: all
all:
	@echo "Please specify 'debug' or 'release'."

.PHONY: debug
debug: CXXFLAGS += -g
debug: DEPFLAGS = -MMD -MP -MF $(DEBUG_OBJDIR)/$*.Td
debug: OBJDIR = $(DEBUG_OBJDIR)
debug: ARCHIVES = ../$(DEBUG_OBJDIR)/$(LIB_NAME)
debug: binary_debug

.PHONY: release
release: CXXFLAGS += -O2
release: DEPFLAGS = -MMD -MP -MF $(RELEASE_OBJDIR)/$*.Td
release: OBJDIR = $(RELEASE_OBJDIR)
release: ARCHIVES = ../$(RELEASE_OBJDIR)/$(LIB_NAME)
release: binary_release


# Define build rules.
# Configure automatic dependency generation as side-effect of compilation.
COMPILE.cpp = $(CXX) $(CXXFLAGS) $(DEPFLAGS) $(SYMBOLS) -c
POSTCOMPILE = mv -f $(OBJDIR)/$*.Td $(OBJDIR)/$*.d

$(DEBUG_OBJDIR)/%.o: %.cpp
$(DEBUG_OBJDIR)/%.o: %.cpp $(DEBUG_OBJDIR)/%.d
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)

$(DEBUG_OBJDIR)/%.d: ;
.PRECIOUS: $(DEBUG_OBJDIR)/%.d

$(RELEASE_OBJDIR)/%.o: %.cpp
$(RELEASE_OBJDIR)/%.o: %.cpp $(RELEASE_OBJDIR)/%.d
	$(COMPILE.cpp) $< -o $@
	$(POSTCOMPILE)

$(RELEASE_OBJDIR)/%.d: ;
.PRECIOUS: $(RELEASE_OBJDIR)/%.d


# Define sources and objects.
SOURCES := $(shell find . -type f -name '*.cpp' -not -name 'Win*')
DEBUG_OBJECTS = $(patsubst ./%,$(DEBUG_OBJDIR)/%,$(SOURCES:.cpp=.o))
RELEASE_OBJECTS = $(patsubst ./%,$(RELEASE_OBJDIR)/%,$(SOURCES:.cpp=.o))


# Define rules.
.PHONY: binary_debug
binary_debug: make_directories $(DEBUG_OBJECTS)
	@echo "[Building debug binary]"
	$(CXX) $(DEBUG_OBJECTS) $(ARCHIVES) $(LIBRARIES) -Wl,-rpath=/usr/local/lib -o ../$(DEBUG_OBJDIR)/$(BINARY_NAME)

.PHONY: binary_release
binary_release: make_directories $(RELEASE_OBJECTS)
	@echo "[Building release binary]"
	$(CXX) $(RELEASE_OBJECTS) $(ARCHIVES) $(LIBRARIES) -Wl,-rpath=/usr/local/lib -o ../$(RELEASE_OBJDIR)/$(BINARY_NAME)

.PHONY: make_directories
make_directories:
	@echo "[Creating directories]"
	@mkdir -p $(OBJDIR)
	@mkdir -p ../$(OBJDIR)

.PHONY: clean
clean:
	@echo "[Cleaning debug and release]"
	@rm -f $(DEBUG_OBJDIR)/*.o
	@rm -f $(DEBUG_OBJDIR)/*.d
	@rm -f ../$(DEBUG_OBJDIR)/$(BINARY_NAME)
	@rm -f $(RELEASE_OBJDIR)/*.o
	@rm -f $(RELEASE_OBJDIR)/*.d
	@rm -f ../$(RELEASE_OBJDIR)/$(BINARY_NAME)


# Include existing dependency files - keep at end of makefile.	
-include $(patsubst %,$(DEBUG_OBJDIR)/%.d,$(basename $(SOURCES)))
-include $(patsubst %,$(RELEASE_OBJDIR)/%.d,$(basename $(SOURCES)))
