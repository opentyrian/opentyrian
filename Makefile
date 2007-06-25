# BUILD SETTINGS ###################################
DEBUG := 1
# Valid values: WINDOWS, UNIX, GP2X
PLATFORM := UNIX
# If building for the GP2X
GP2X_CHAINPREFIX := /opt/open2x/gcc-4.1.1-glibc-2.3.6
GP2X_CHAIN := $(GP2X_CHAINPREFIX)/bin/arm-open2x-linux-

# END SETTINGS #####################################

TARGET := tyrian
OBJS := animlib.o backgrnd.o config.o destruct.o editship.o episodes.o error.o fm_synth.o fmopl.o fonthand.o helptext.o joystick.o jukebox.o keyboard.o lds_play.o loudness.o lvllib.o lvlmast.o mainint.o musmast.o network.o newshape.o nortsong.o nortvars.o opentyr.o pallib.o params.o pcxload.o pcxmast.o picload.o picload2.o scroller.o setup.o shplib.o shplib2.o shpmast.o sndmast.o starfade.o starlib.o tyrian2.o varz.o vga256d.o 

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0
else
	DEBUG_FLAGS := -g -O2 -fomit-frame-pointer -DNDEBUG -fno-strict-aliasing
endif

ifeq ($(PLATFORM), WINDOWS)
	SDL_CFLAGS := -I/mingw/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
	SDL_LDFLAGS := -L/mingw/lib -lmingw32 -lSDLmain -lSDL -mwindows
endif
ifeq ($(PLATFORM), GP2X)
	SDL_CFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --cflags` -I$(GP2X_CHAINPREFIX)/include -DTARGET_GP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math -funroll-loops -std=c99
	SDL_LDFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --libs` -static -L$(GP2X_CHAINPREFIX)/lib
	CC := $(GP2X_CHAIN)gcc
	STRIP := $(GP2X_CHAIN)strip
endif
ifeq ($(PLATFORM), UNIX)
	SDL_CFLAGS := $(shell sdl-config --cflags)
	SDL_LDFLAGS := $(shell sdl-config --libs)
endif


CFLAGS := -ansi -pedantic -Wall -Wstrict-prototypes -Wold-style-definition -Wmissing-declarations -Wno-unused -Werror -I$(CURDIR)/src/ $(DEBUG_FLAGS) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LDFLAGS) -lm

####################################################

all : $(TARGET)

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

obj/%.d : src/%.c
	@echo 'Generating dependencies for file $<'
	@set -e; $(CC) -MM -MT obj/$*.o $(CFLAGS) $< \
	    | sed 's/obj\/\($*\)\.o[ :]*/obj\/\1.o obj\/$*\.d : /g' > $@; \
	    [ -s $@ ] || rm -f $@

obj/%.o : src/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

.PHONY : clean

clean :
	rm -f obj/*.o obj/*.d
	rm -f $(TARGET)
