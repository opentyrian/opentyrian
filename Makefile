# BUILD SETTINGS ###################################
DEBUG := 1

# END SETTINGS #####################################

TARGET := tyrian
OBJS := opentyr.o newshape.o fonthand.o

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0 -Wno-unused
else
	DEBUG_FLAGS := -O2 -fomit-frame-pointer -DNDEBUG
endif

SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CFLAGS := -ansi -pedantic -Wall -Werror -I$(CURDIR)/src/ $(DEBUG_FLAGS) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LDFLAGS) -lm

####################################################

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o : src/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY : clean

clean :
	rm -f obj/*.o
	rm -f $(TARGET)
