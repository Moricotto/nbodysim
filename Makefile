CC	 = g++
CCFLAGS = -std=c++20 -Wall -MMD
DBGCFLAGS = -ggdb -O0 -g -no-pie
RELCFLAGS = -O3 -D NDEBUG
CFLAGS = $(DBGCFLAGS) $(CCFLAGS)
LFLAGS = $(DBGCFLAGS) $(CCFLAGS)
LIBS = 

# Source files
FILES = main
SOURCES = $(addsuffix .cpp,$(addprefix $(SRCDIR)/,$(FILES)))
OBJECTS = $(addsuffix .o,$(addprefix $(BUILDDIR)/,$(FILES)))
DEPS = $(addsuffix .d,$(addprefix $(BUILDDIR)/,$(FILES)))
TARGET = nbody.exe

# Subdirectories
SRCDIR = src
BUILDDIR = obj
TARGETDIR = bin
LIBDIR = lib
HEADERS = 

$(TARGETDIR)/$(TARGET): $(OBJECTS) 
	$(CC) $(LFLAGS) -o $(TARGETDIR)/$(TARGET) $^ -L$(LIBDIR) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.hpp
	$(CC) $(CFLAGS) $(HEADERS) -c -o $@ $<

# Make directories for compilation
directories: 
	mkdir -p $(BUILDDIR)
	mkdir -p $(TARGETDIR)

-include $(DEPS)

clean:
	rm -rf $(BUILDDIR)/*
	rm $(TARGETDIR)/$(TARGET)

.PHONY: clean directories