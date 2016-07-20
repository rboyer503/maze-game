# Adapted from http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/.

# Initialize variables.
CXXFLAGS := -std=c++11
SYMBOLS := -D _LINUX -D GCC_BUILD
LIB_NAME := MazeShared.a
DEBUG_OBJDIR := x64/gccDebug
RELEASE_OBJDIR := x64/gccRelease


# Define top-level rules.
.PHONY: all
all:
	@echo "Please specify 'debug' or 'release'."

.PHONY: debug
debug: CXXFLAGS += -g
debug: DEPFLAGS = -MMD -MP -MF $(DEBUG_OBJDIR)/$*.Td
debug: OBJDIR = $(DEBUG_OBJDIR)
debug: library_debug

.PHONY: release
release: CXXFLAGS += -O2
release: DEPFLAGS = -MMD -MP -MF $(RELEASE_OBJDIR)/$*.Td
release: OBJDIR = $(RELEASE_OBJDIR)
release: library_release


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
SOURCES := $(shell find . -type f -name '*.cpp')
DEBUG_OBJECTS = $(patsubst ./%,$(DEBUG_OBJDIR)/%,$(SOURCES:.cpp=.o))
RELEASE_OBJECTS = $(patsubst ./%,$(RELEASE_OBJDIR)/%,$(SOURCES:.cpp=.o))


# Define rules.
.PHONY: library_debug
library_debug: make_directories $(DEBUG_OBJECTS)
	@echo "[Building debug library]"
	ar rvs ../$(DEBUG_OBJDIR)/$(LIB_NAME) $(DEBUG_OBJECTS)

.PHONY: library_release
library_release: make_directories $(RELEASE_OBJECTS)
	@echo "[Building release library]"
	ar rvs ../$(RELEASE_OBJDIR)/$(LIB_NAME) $(RELEASE_OBJECTS)

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
	@rm -f ../$(DEBUG_OBJDIR)/*.a
	@rm -f $(RELEASE_OBJDIR)/*.o
	@rm -f $(RELEASE_OBJDIR)/*.d
	@rm -f ../$(RELEASE_OBJDIR)/*.a


# Include existing dependency files - keep at end of makefile.	
-include $(patsubst %,$(DEBUG_OBJDIR)/%.d,$(basename $(SOURCES)))
-include $(patsubst %,$(RELEASE_OBJDIR)/%.d,$(basename $(SOURCES)))
