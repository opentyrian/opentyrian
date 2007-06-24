# BUILD SETTINGS ###################################
DEBUG := 1
# Valid values: WINDOWS, UNIX, GP2X
PLATFORM := UNIX
PROFILE := 0
# If building for the GP2X
GP2X_DEVKIT := /stuff/devkit-gp2x
GP2X_CC := gp2x-gcc
GP2X_STRIP := gp2x-strip

# END SETTINGS #####################################

TARGET := tyrian
OBJS := adlibemu.o animlib.o backgrnd.o config.o destruct.o editship.o episodes.o error.o fm_synth.o fonthand.o helptext.o joystick.o jukebox.o keyboard.o lds_play.o loudness.o lvllib.o lvlmast.o mainint.o musmast.o network.o newshape.o nortsong.o nortvars.o opentyr.o pallib.o params.o pcxload.o pcxmast.o picload.o picload2.o scroller.o setup.o shplib.o shplib2.o shpmast.o sndmast.o starfade.o starlib.o tyrian2.o varz.o vga256d.o 

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0
else
	DEBUG_FLAGS := -g -O2 -fomit-frame-pointer -DNDEBUG -fno-strict-aliasing
endif

ifeq ($(PLATFORM), WINDOWS)
	SDL_CFLAGS := -I/mingw/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
	SDL_LDFLAGS := -L/mingw/lib -lmingw32 -lSDLmain -lSDL -mwindows
else ifeq ($(PLATFORM), GP2X)
	SDL_CFLAGS := -I$(GP2X_DEVKIT)/include/SDL -D_GNU_SOURCE=1 -DTARGET_GP2X
	SDL_LDFLAGS := -L$(GP2X_DEVKIT)/lib -lpthread -lSDL -static
	CC := $(GP2X_CC)
	STRIP := $(GP2X_STRIP)
else
	SDL_CFLAGS := $(shell sdl-config --cflags)
	SDL_LDFLAGS := $(shell sdl-config --libs)
endif


CFLAGS := -ansi -pedantic -Wall -Wstrict-prototypes -Wold-style-definition -Wmissing-declarations -Wno-unused -Werror -I$(CURDIR)/src/ $(DEBUG_FLAGS) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LDFLAGS) -lm

ifeq ($(PROFILE), 1)
	CFLAGS += -pg
	LDFLAGS += -pg
endif

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
