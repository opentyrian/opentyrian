# BUILD SETTINGS ###################################

# Valid values: WINDOWS, UNIX, GP2X
PLATFORM := UNIX

# If building for the GP2X
GP2X_CHAINPREFIX := /opt/open2x/gcc-4.1.1-glibc-2.3.6
GP2X_CHAIN := $(GP2X_CHAINPREFIX)/bin/arm-open2x-linux-

# END SETTINGS #####################################

TARGET := tyrian
OBJS := animlib.o backgrnd.o config.o destruct.o editship.o episodes.o file.o fm_synth.o fmopl.o fonthand.o game_menu.o helptext.o joystick.o jukebox.o keyboard.o lds_play.o loudness.o lvllib.o lvlmast.o mainint.o menus.o mtrand.o musmast.o network.o newshape.o nortsong.o nortvars.o opentyr.o palette.o params.o pcxload.o pcxmast.o picload.o scroller.o setup.o sndmast.o starlib.o sizebuf.o tyrian2.o varz.o vga256d.o vga_palette.o video.o video_scale.o xmas.o

STRIP := strip

CFLAGS += --std=c99 -pedantic -Wall -Wstrict-prototypes -Wold-style-definition -Wmissing-declarations -I$(CURDIR)/src/
LDFLAGS += -lm

ifeq ($(PLATFORM), UNIX)
	CFLAGS += -DTARGET_UNIX
	
	SDL_CFLAGS := $(shell sdl-config --cflags)
	SDL_LDFLAGS := $(shell sdl-config --libs) -lSDL_net
endif
ifeq ($(PLATFORM), WINDOWS)
	SDL_CFLAGS := -I/mingw/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
	SDL_LDFLAGS := -L/mingw/lib -lmingw32 -lSDLmain -lSDL -lSDL_net -mwindows
endif
ifeq ($(PLATFORM), GP2X)
	CC := $(GP2X_CHAIN)gcc
	STRIP := $(GP2X_CHAIN)strip
	
	CFLAGS += -DTARGET_GP2X -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math
	
	SDL_CFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --cflags` -I$(GP2X_CHAINPREFIX)/include
	SDL_LDFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --libs` -L$(GP2X_CHAINPREFIX)/lib
endif

CFLAGS += $(SDL_CFLAGS)
LDFLAGS += $(SDL_LDFLAGS)

DEBUG := 1

DEBUG_FLAGS_0 := -g -O2 -DNDEBUG -fno-strict-aliasing
DEBUG_FLAGS_1 := -g3 -O0 -Werror

SVN_REV := $(shell svnversion src -n)
ifneq ($(SVN_REV), )
	ifeq ($(SVN_REV), exported)
		SVN_REV := unknown
	endif
	
	CFLAGS += -DSVN_REV=\"$(SVN_REV)\"
endif

####################################################

all : $(TARGET)

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

obj/%.d : obj/%.o
obj/%.o : src/%.c
	$(CC) -o $@ -MMD -c $(DEBUG_FLAGS_$(DEBUG)) $(CFLAGS) $<

.PHONY : release clean

release : DEBUG := 0
release : all
	$(STRIP) $(TARGET)

clean :
	rm -f obj/*.o obj/*.d
	rm -f $(TARGET)
