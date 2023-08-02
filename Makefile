# The name of the source files
SOURCES = src/int.c src/web.c src/modbus.c src/main.c
OBJ_DIR = src
# The name of the executable
EXE = main.out

# Flags for compilation (adding warnings are always good)
CFLAGS = -Wall -Wextra -Wundef -pedantic -Wno-missing-field-initializers -DWEB -DMODBUS

# Flags for linking (none for the moment)
LDFLAGS =

# Libraries to link with (none for the moment)
LIBS = -lm

# Use the GCC frontend program when linking
LD = gcc

# This creates a list of object files from the source files
OBJECTS = $(SOURCES:%.c=%.o)

df = $(OBJ_DIR)/$(*F)

# The first target, this will be the default target if none is specified
# This target tells "make" to make the "all" target
default: all

# Having an "all" target is customary, so one could write "make all"
# It depends on the executable program
all: $(EXE) ctest test

# This will link the executable from the object files
$(EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o  $(EXE) $(LIBS)

# This is a target that will compiler all needed source files into object files
# We don't need to specify a command or any rules, "make" will handle it automatically
%.o: %.c

# Target to clean up after us
clean:
	-rm -f $(EXE)      # Remove the executable file
	-rm -f $(OBJECTS)  # Remove the object files

# Finally we need to tell "make" what source and header file each object file depends on
#hellomain.o: hellomain.c helloheader.h
#hellofunc.o: hellofunc.c helloheader.h
$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.c
	@# Build the dependency file
	@$(CXX) -MM -MP -MT $(df).o -MT $(df).d $(CFLAGS) $< > $(df).d
	@# Compile the object file
	@echo " C: " $< " => " $@
	@$(CXX) -c $< $(CFLAGS) -o $@

ctest:
	gcc -O0 -Isrc -o test.out tests/test.c
	./test.out

test:
	./test.sh

webtest:
	./main.out '( WEB SERVER test ) 8080 web 10 FOR 1000 ms ; ;'
	./main.out '( MODBUS SERVER test ) 5002 mbs 10 FOR 1000 ms ; ;

valgrind:
	valgrind -q -s --leak-check=full ./main-val.out '8080 web 0 0 set 10000 FOR dup 0 get + 0 set ; 0 get . ;'