# BUILD SETTINGS ###########################################

ifndef PLATFORM
    ifneq ($(filter Msys Cygwin,$(shell uname -o)),)
        PLATFORM := WIN32
    else
        PLATFORM := UNIX
    endif
endif

TARGET := opentyrian
WITH_NETWORK := 0

############################################################

STRIP := strip
SDL_CONFIG := sdl2-config

include crosscompile.mk

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)

# FLAGS ####################################################

ifneq ($(MAKECMDGOALS), release)
    EXTRA_CFLAGS += -g3 -O0 -Werror
else
    EXTRA_CFLAGS += -g0 -O2 -DNDEBUG
endif
EXTRA_CFLAGS += -MMD -pedantic -Wall -Wextra -Wno-missing-field-initializers
ifeq ($(WITH_NETWORK),1)
    EXTRA_CFLAGS += -DWITH_NETWORK
endif

HG_REV := $(shell hg id -ib && touch src/hg_revision.h)
ifneq ($(HG_REV), )
    EXTRA_CFLAGS += '-DHG_REV="$(HG_REV)"'
endif

EXTRA_LDLIBS += -lm

SDL_CFLAGS := $(shell $(SDL_CONFIG) --cflags)
SDL_LDLIBS := $(shell $(SDL_CONFIG) --libs)
ifeq ($(WITH_NETWORK),1)
    SDL_LDLIBS += -lSDL_net
endif

ALL_CFLAGS += -std=c99 -I./src -DTARGET_$(PLATFORM) $(EXTRA_CFLAGS) $(SDL_CFLAGS) $(CFLAGS)
ALL_LDFLAGS += $(LDFLAGS)
LDLIBS += $(EXTRA_LDLIBS) $(SDL_LDLIBS)

# RULES ####################################################

.PHONY : all release clean

all : $(TARGET)

release : all
	$(STRIP) $(TARGET)

clean :
	rm -rf obj/*
	rm -f $(TARGET)

ifneq ($(MAKECMDGOALS), clean)
    -include $(OBJS:.o=.d)
endif

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(ALL_LDFLAGS) $^ $(LDLIBS)

obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) -c -o $@ $(ALL_CFLAGS) $<

