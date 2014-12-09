
CC := g++ # This is the main compiler
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := ../../../source
BUILDDIR := ../../../../build/vault/unix
TARGET := bin/runner
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT) | grep -v '_mac' | grep -v '_win')
INCLUDE_DIRS = $(shell find $(SRCDIR) -type d | grep -v '_mac' | grep -v '_win')
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g # -Wall
LIB := -pthread
INC := \
  -I /usr/include/i386 \
  -I .. \
  -I $(SRCDIR)/vtypes \
  -I $(SRCDIR)/vtypes/_unix \
  -I $(SRCDIR)/containers \
  -I $(SRCDIR)/containers/_unix \
  -I $(SRCDIR)/files \
  -I $(SRCDIR)/files/_unix \
  -I $(SRCDIR)/server \
  -I $(SRCDIR)/sockets \
  -I $(SRCDIR)/sockets/_unix \
  -I $(SRCDIR)/streams \
  -I $(SRCDIR)/threads \
  -I $(SRCDIR)/threads/_unix \
  -I $(SRCDIR)/toolbox \
  -I $(SRCDIR)/unittest \

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean