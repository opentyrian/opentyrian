# BUILD SETTINGS ###########################################

PLATFORM := UNIX

# CROSS-COMPILE SETTINGS ###################################

UNIX_PREFIX := /usr
UNIX_EPREFIX := /usr
UNIX_HOST := 

WIN32_PREFIX := /usr/i486-mingw32
WIN32_EPREFIX := $(UNIX_EPREFIX)
WIN32_HOST := i486-mingw32
WIN32_EXT := .exe

GP2X_PREFIX := /opt/open2x/gcc-4.1.1-glibc-2.3.6
GP2X_EPREFIX := $(GP2X_PREFIX)
GP2X_HOST := arm-open2x-linux
GP2X_EXT := .gpe
GP2X_CFLAGS := -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math

# END SETTINGS #############################################

TARGET := tyrian$($(PLATFORM)_EXT)

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)

# TOOLCHAIN ################################################

CC := gcc
STRIP := strip

PREFIX := $($(PLATFORM)_PREFIX)
EPREFIX := $($(PLATFORM)_EPREFIX)
HOST := $($(PLATFORM)_HOST)

BINDIR := $(EPREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

ifneq ($(HOST), )
	CC := $(HOST)-$(CC)
	STRIP := $(HOST)-$(STRIP)
endif

CC := $(BINDIR)/$(CC)
STRIP := $(BINDIR)/$(STRIP)

# FLAGS ####################################################

NDEBUG_FLAGS := -g0 -O2 -DNDEBUG
DEBUG_FLAGS := -g3 -O0 -Werror

CFLAGS += --std=c99 -pedantic -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers
CFLAGS += -I./src -I$(INCLUDEDIR)
LDFLAGS += -L$(LIBDIR) -lm

ifneq ($(PREFIX), )
	SDL_CONFIG_PREFIX := $(PREFIX)/bin
else
	SDL_CONFIG_PREFIX := $(BINDIR)
endif

SDL_CFLAGS := $(shell $(SDL_CONFIG_PREFIX)/sdl-config --cflags)
SDL_LDFLAGS := $(shell $(SDL_CONFIG_PREFIX)/sdl-config --libs) -lSDL_net

CFLAGS += $($(PLATFORM)_CFLAGS) -DTARGET_$(PLATFORM) $(SDL_CFLAGS)
LDFLAGS += $($(PLATFORM)_LDFLAGS) $(SDL_LDFLAGS)

HG_REV := $(shell hg id -ib && touch src/hg_revision.h)
ifneq ($(HG_REV), )
	CFLAGS += -DHG_REV="\"$(HG_REV)\""
endif

# RULES ####################################################

.PHONY : all
all : $(TARGET)

.PHONY : release
release : DEBUG_FLAGS := $(NDEBUG_FLAGS)
release : all
	$(STRIP) $(TARGET)

.PHONY : clean
clean :
	rm -rf obj/*
	rm -f $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

obj/%.d : obj/%.o
obj/%.o : src/%.c
	@mkdir -p "$(dir $@)"
	$(CC) -o $@ -MMD -c $(DEBUG_FLAGS) $(CFLAGS) $< 
