# CROSS-COMPILE SETTINGS ###################################

WIN32_PREFIX := /usr/i486-mingw32
WIN32_EPREFIX := /usr
WIN32_HOST := i486-mingw32
WIN32_EXT := .exe

GP2X_PREFIX := /opt/open2x/gcc-4.1.1-glibc-2.3.6
GP2X_EPREFIX := $(GP2X_PREFIX)
GP2X_HOST := arm-open2x-linux
GP2X_EXT := .gpe
GP2X_CFLAGS := -mcpu=arm920t -mtune=arm920t -msoft-float -ffast-math

DINGUX_PREFIX := /opt/mipsel-linux-uclibc/usr
DINGUX_EPREFIX := $(DINGUX_PREFIX)
DINGUX_HOST := mipsel-linux-uclibc
DINGUX_EXT := .dge

# END SETTINGS #############################################

CC := gcc

TARGET := $(TARGET)$($(PLATFORM)_EXT)

ifdef CROSS

PREFIX := $($(PLATFORM)_PREFIX)
EPREFIX := $($(PLATFORM)_EPREFIX)
HOST := $($(PLATFORM)_HOST)

BINDIR := $(EPREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include

ifneq ($(HOST), )
    CC := $(HOST)-$(notdir $(CC))
    STRIP := $(HOST)-$(notdir $(STRIP))
endif

CC := $(BINDIR)/$(notdir $(CC))
STRIP := $(BINDIR)/$(notdir $(STRIP))

SDL_CONFIG := $(PREFIX)/bin/$(SDL_CONFIG)

# FLAGS ####################################################

ALL_CFLAGS += -I$(INCLUDEDIR) $($(PLATFORM)_CFLAGS)
ALL_LDFLAGS += -L$(LIBDIR) $($(PLATFORM)_LDFLAGS)

endif

